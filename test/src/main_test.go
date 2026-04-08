package main

import (
	"log"
	"os"
	"testing"
	"time"

	"github.com/gorilla/websocket"
)

const (
	StatusOk uint8 = iota
	StatusError
	StatusInitial = 255

	REQ_SIZE = 8
	RES_SIZE = 7
)

var (
	conn    *websocket.Conn
	err     error
	ID      uint8
	MsgChan = make(chan []uint8)
)

func read(conn *websocket.Conn) {
	for {
		_, msg, err := conn.ReadMessage()
		if err != nil {
			log.Fatal(err)
		}

		MsgChan <- msg
	}
}

// func SendCommand(t *testing.T, msg []uint8, _log ...bool) []uint8 {
// 	t.Helper()

// 	start := time.Now()
// 	if err := conn.WriteMessage(websocket.BinaryMessage, msg); err != nil {
// 		t.Fatal(err)
// 	}
// 	res := <-MsgChan
// 	rtt := time.Since(start)

// 	if len(_log) > 0 && _log[0] {
// 		t.Logf("req=%v rtt=%v", msg, rtt)
// 	}

// 	return res
// }

func SendCommand(t *testing.T, data []uint8, _log ...bool) [][]uint8 {
	t.Helper()

	reqCount := (len(data) / REQ_SIZE)
	res := make([][]uint8, reqCount)

	start := time.Now()
	if err := conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
		t.Fatal(err)
	}
	for i := range reqCount {
		res[i] = <-MsgChan
	}
	rtt := time.Since(start)

	if len(_log) > 0 && _log[0] {
		t.Logf("req=%v, res=%v, rtt=%v", data, res, rtt)
	}

	return res
}

func TestMain(m *testing.M) {
	// conn, _, err = websocket.DefaultDialer.Dial("ws://localhost/commander_vendor", nil)
	conn, _, err = websocket.DefaultDialer.Dial("ws://192.168.4.1/ws", nil)
	if err != nil {
		log.Fatal(err)
	}

	go read(conn)

	if err := conn.WriteJSON(map[string]string{"token": "TOKEN_A"}); err != nil {
		log.Fatal(err)
	}

	// STATUS_INITIAL
	h := <-MsgChan
	// log.Println(h)
	ID = h[1]

	code := m.Run()

	conn.Close()

	os.Exit(code)
}

func Chunk64(data []byte) [][]byte {
	const size = 64
	var chunks [][]byte
	for i := 0; i < len(data); i += size {
		end := min(i+size, len(data))
		chunks = append(chunks, data[i:end])
	}
	return chunks
}
