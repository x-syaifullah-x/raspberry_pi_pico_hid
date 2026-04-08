package ws

import (
	"io"
	"log"

	"github.com/gorilla/websocket"
)

type Client struct {
	ID   uint8
	Conn *websocket.Conn
	Send chan []uint8
	Hub  *Hub
}

func (c *Client) ReadPump(w io.Writer) {
	defer func() {
		c.Hub.Unregister <- c
		c.Conn.Close()
	}()

	for {
		_, msg, err := c.Conn.ReadMessage()
		if err != nil {
			if !websocket.IsCloseError(err, websocket.CloseNormalClosure, websocket.CloseGoingAway) {
				log.Println("closed unexpectedly:", err)
			}
			break
		}

		// nano := time.Now().UnixNano()
		// req_id := uint64(nano) ^ rand.Uint64()

		if _, err = w.Write(msg); err != nil {
			c.Send <- []uint8{StatusErr, CMD_UNKNOWN, 255, 0, 0, 0, 0}
			continue
		}
	}
}

func (c *Client) WritePump() {
	defer c.Conn.Close()

	for msg := range c.Send {
		c.Conn.WriteMessage(websocket.BinaryMessage, msg)
	}
}
