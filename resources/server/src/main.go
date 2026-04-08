package main

import (
	"encoding/json"
	"errors"
	"log"
	"net/http"
	"os"

	"github.com/gorilla/websocket"
)

type EventType string

const (
	EVENT_INITIALIZE EventType = "INITIALIZE"
	EVENT_UPDATED    EventType = "UPDATED"
)

var (
	gpios       = []uint8{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28}
	device, err = os.OpenFile(os.Args[1], os.O_RDWR, 0)
)

type PiPicoData struct {
	Methode *uint8 `json:"methode,omitempty"`
	Gpio    *uint8 `json:"gpio,omitempty"`
	Value   uint16 `json:"value"`
}

type WsResponseError struct {
	Message string `json:"message"`
}

type WSResponse struct {
	Type    EventType        `json:"type,omitempty"`
	Result  *PiPicoData      `json:"result,omitempty"`
	Results []PiPicoData     `json:"results,omitempty"`
	Error   *WsResponseError `json:"error,omitempty"`
}

type Client struct {
	conn *websocket.Conn
	send chan []byte
	hub  *Hub
}

func u16(b []byte, i int) uint16 {
	return uint16(b[i]) | uint16(b[i+1])<<8
}

// func u64(b []byte, i int) uint64 {
// 	return uint64(b[i]) | uint64(b[i+1])<<8 | uint64(b[i+2])<<16 | uint64(b[i+3])<<24 | uint64(b[i+4])<<32 | uint64(b[i+5])<<40 | uint64(b[i+6])<<48 | uint64(b[i+7])<<56
// }

func deviceRequest(device *os.File, args ...uint8) (*PiPicoData, error) {
	_, err := device.Write(args)
	if err != nil {
		return nil, err
	}

	buf := make([]byte, 5 /* hid_report.h : 0x95, sizeof(gpio) */)
	_, err = device.Read(buf)
	if err != nil {
		return nil, err
	}

	error := buf[0]
	if error > 0 {
		switch error {
		case 0x01:
			return nil, errors.New("INVALID_METHODE_NUMBER")
		case 0x02:
			return nil, errors.New("INVALID_METHOD_PARAM")
		default:
			return nil, errors.New("UNKNOWN_ERROR")
		}

	}
	gpio := &buf[2]
	if *gpio > byte(len(gpios)-1) {
		gpio = nil
	}

	return &PiPicoData{Methode: &buf[1], Gpio: gpio, Value: u16(buf, 3)}, nil
}

func (c *Client) readPump() {
	defer func() {
		c.hub.unregister <- c
		c.conn.Close()
	}()

	for {
		_, msg, err := c.conn.ReadMessage()
		if err != nil {
			break
		}

		if len(msg) == 0 {
			continue
		}

		var response WSResponse

		val, err := deviceRequest(device, msg...)
		if err != nil {
			response = WSResponse{Error: &WsResponseError{Message: err.Error()}}
		} else {
			response = WSResponse{Type: EVENT_UPDATED, Result: val}
		}

		responseJson, _ := json.Marshal(response)

		if response.Error != nil {
			c.conn.WriteMessage(websocket.TextMessage, responseJson)
			continue
		}

		c.hub.broadcast <- responseJson
	}
}

func (c *Client) writePump() {
	defer c.conn.Close()
	for msg := range c.send {
		c.conn.WriteMessage(websocket.TextMessage, msg)
	}
}

type Hub struct {
	clients    map[*Client]bool
	broadcast  chan []byte
	register   chan *Client
	unregister chan *Client
}

func NewHub() *Hub {
	return &Hub{
		clients:    make(map[*Client]bool),
		broadcast:  make(chan []byte),
		register:   make(chan *Client),
		unregister: make(chan *Client),
	}
}

func (h *Hub) Run() {
	for {
		select {
		case c := <-h.register:
			var results []PiPicoData
			for index := range gpios {
				data, err := deviceRequest(device, 0x00, gpios[index])
				if err != nil {
					log.Panic(err.Error())
				}
				results = append(results, PiPicoData{Gpio: data.Gpio, Value: data.Value})
			}

			response, _ := json.Marshal(WSResponse{Type: EVENT_INITIALIZE, Results: results})
			c.conn.WriteMessage(websocket.TextMessage, response)
			h.clients[c] = true

		case c := <-h.unregister:
			if _, ok := h.clients[c]; ok {
				delete(h.clients, c)
				close(c.send)
			}

		case msg := <-h.broadcast:
			for c := range h.clients {
				select {
				case c.send <- msg:
				default:
					close(c.send)
					delete(h.clients, c)
				}
			}
		}
	}
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func serveWS(hub *Hub, w http.ResponseWriter, r *http.Request) {
	conn, _ := upgrader.Upgrade(w, r, nil)

	client := &Client{
		conn: conn,
		send: make(chan []byte, 256),
		hub:  hub,
	}

	hub.register <- client

	go client.writePump()
	go client.readPump()
}

func main() {
	if err != nil {
		log.Panic(err)
	}
	defer device.Close()

	hub := NewHub()
	go hub.Run()

	http.HandleFunc("/raspberry_pi_pico", func(w http.ResponseWriter, r *http.Request) { serveWS(hub, w, r) })

	log.Println("Server running on :8080")
	err = http.ListenAndServe(":8080", nil)
	if err != nil {
		log.Fatal(err)
	}
}
