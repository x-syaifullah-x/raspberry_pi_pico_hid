package commander

import (
	"commander/src/ws"
	"errors"
	"io"
	"log"
	"os"

	"github.com/google/gousb"
	"github.com/gorilla/websocket"
)

const (
	stateSize = 5
)

var (
	state = make(map[uint8][stateSize]uint8)
)

func sendResponse(hub *ws.Hub, buf []uint8) {
	// count := min(len(buf)>>3, ws.PacketSize)
	// for i := 0; i < count*ws.PacketSize; i += ws.PacketSize {
	lenBuf := len(buf)
	total := lenBuf - (lenBuf % ws.RequestSize)
	for i := 0; i < total; i += ws.RequestSize {
		id := buf[i]
		status := buf[i+1]
		cmd := buf[i+2]
		raws := [stateSize]uint8{buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7]}
		res := []uint8{status, cmd, raws[0], raws[1], raws[2], raws[3], raws[4]}

		if status == ws.StatusErr {
			if client := hub.Clients[id]; client != nil {
				client.Send <- res
			}
			continue
		}

		state[cmd] = raws

		hub.Broadcast <- res
	}
}

func Listen(hub *ws.Hub, writer io.Writer, reader io.Reader) {
	commands := []uint8{
		ws.ID_HOST, ws.CMD_LED_DEFAULT, 255, 0, 0, 0, 0, 0,
		ws.ID_HOST, ws.CMD_ADC_DMA, 255, 0, 0, 0, 0, 0,
		ws.ID_HOST, ws.CMD_TB6612FNG_POWER, 255, 0, 0, 0, 0, 0,
		ws.ID_HOST, ws.CMD_TB6612FNG_MOTOR_STATE, ws.CMD_TB6612FNG_MOTOR_1, 0, 0, 0, 0, 0,
		ws.ID_HOST, ws.CMD_TB6612FNG_MOTOR_STATE, ws.CMD_TB6612FNG_MOTOR_2, 0, 0, 0, 0, 0,
	}
	n := len(commands)
	chunkSize := ws.RxSize
	buf := make([]uint8, ws.RxSize)
	for i := 0; i < n; i += chunkSize {
		end := min(i+chunkSize, n)
		writer.Write(commands[i:end])
		if n, _ := reader.Read(buf); n > 0 {
			sendResponse(hub, buf[:n])
		}
	}
	for {
		n, err := reader.Read(buf)
		if err != nil {
			switch {
			case errors.Is(err, gousb.TransferOverflow):
				log.Printf("gousb.TransferOverflow: %v", err)
				continue
			}
			for _, client := range hub.Clients {
				client.Send <- websocket.FormatCloseMessage(websocket.CloseGoingAway, "Device disconnected")
			}
			log.Printf("reader.Read failed: %v", err)
			os.Exit(0)
		}

		if n == 0 {
			continue
		}

		sendResponse(hub, buf[:n])
	}
}
