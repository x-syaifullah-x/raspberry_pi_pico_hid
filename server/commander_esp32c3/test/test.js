const ws = new WebSocket("ws://192.168.4.1/ws")

ws.binaryType = "arraybuffer";

ws.onclose = (e) => {
  console.log("Server closed:", e);
}

ws.onopen = (x) => {
  ws.send(JSON.stringify({
    token: "TOKEN_A",
  }));
};

ws.onmessage = (event) => {
  console.log("Message from server:", event.data);
}

ws.send(new Uint8Array([2, 21, 0, 0, 0, 0, 0, 0]))