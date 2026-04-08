const ws = new WebSocket("ws://localhost:8080/raspberry_pi_pico")
ws.onmessage = (event) => {
  console.log("Message from server:", event.data);
}

// ws.send(new Uint8Array([value]))