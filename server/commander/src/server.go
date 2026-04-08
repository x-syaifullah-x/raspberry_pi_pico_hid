package commander

import (
	"embed"
	"html/template"
	"io"
	"io/fs"
	"net"
	"net/http"
	"time"

	"commander/src/ws"

	"github.com/gorilla/websocket"
)

var (
	dummyData = map[string]uint8{"TOKEN_A": ws.ID_A, "TOKEN_B": ws.ID_B}
	upgrader  = websocket.Upgrader{CheckOrigin: func(r *http.Request) bool { return true }}
)

//go:embed templates/*.html
var content embed.FS

var staticFS, _ = fs.Sub(content, "templates")

var tmpl = template.Must(template.ParseFS(staticFS, "index.html"))

func ControlPanel(w http.ResponseWriter, r *http.Request) {
	data := struct {
		Host string
	}{
		Host: r.Host,

	}

	tmpl.Execute(w, data)
}

func ServeWS(hub *ws.Hub, w http.ResponseWriter, r *http.Request, writer io.Writer) {
	conn, _ := upgrader.Upgrade(w, r, w.Header())
	conn.SetReadDeadline(time.Now().Add(5 * time.Second))
	var auth ws.Auth
	err := conn.ReadJSON(&auth)
	if err != nil {
		if ne, ok := err.(net.Error); ok && ne.Timeout() {
			conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.ClosePolicyViolation, "Authentication timeout (no response within 5 seconds)"))
		} else {
			conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseUnsupportedData, "Invalid JSON payload for authentication"))
		}
		return
	}
	conn.SetReadDeadline(time.Time{})

	id := dummyData[auth.Token]
	if id == 0 {
		conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.ClosePolicyViolation, "Missing token"))
		return
	}

	if hub.Clients[id] != nil {
		conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.ClosePolicyViolation, "Token already in use"))
		return
	}

	const HeaderSize = ws.ResponseSize
	const RecordSize = ws.ResponseSize
	data := make([]byte, HeaderSize+RecordSize*len(state))
	data[0] = ws.StatusInitial
	data[1] = id
	i := HeaderSize
	for cmd, s := range state {
		data[i+0] = ws.StatusOk
		data[i+1] = cmd
		data[i+2] = s[0]
		data[i+3] = s[1]
		data[i+4] = s[2]
		data[i+5] = s[3]
		data[i+6] = s[4]
		i += RecordSize
	}
	conn.WriteMessage(websocket.BinaryMessage, data)

	client := &ws.Client{
		ID:   id,
		Conn: conn,
		Send: make(chan []uint8, 16),
		Hub:  hub,
	}

	hub.Register <- client

	go client.WritePump()
	go client.ReadPump(writer)
}
