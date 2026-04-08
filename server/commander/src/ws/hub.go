package ws

type Hub struct {
	Clients    map[uint8]*Client
	Broadcast  chan []uint8
	Register   chan *Client
	Unregister chan *Client
}

func NewHub() *Hub {
	return &Hub{
		Clients:    make(map[uint8]*Client),
		Broadcast:  make(chan []uint8),
		Register:   make(chan *Client),
		Unregister: make(chan *Client),
	}
}

func (h *Hub) Run() {
	for {
		select {
		case c := <-h.Register:
			h.Clients[c.ID] = c

		case c := <-h.Unregister:
			if _, ok := h.Clients[c.ID]; ok {
				delete(h.Clients, c.ID)
				close(c.Send)
			}

		case msg := <-h.Broadcast:
			for _, client := range h.Clients {
				select {
				case client.Send <- msg:
				default:
					close(client.Send)
					delete(h.Clients, client.ID)
				}
			}
		}
	}
}
