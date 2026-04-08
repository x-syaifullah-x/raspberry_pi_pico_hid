package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	commander "commander/src"
	"commander/src/ws"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
)

const (
	Port = ":8080"
)

func main() {
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGTERM, syscall.SIGINT)

	go func() {
		<-sig
		os.Exit(0)
	}()

	vid := flag.String("vid", "", "USB vendor id")
	pid := flag.String("pid", "", "USB product id")
	serialNumber := flag.String("serial", "", "device serial")
	flag.Parse()

	var serialPort serial.Port
	defer func() {
		serialPort.Close()
	}()
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		log.Fatal(err)
	}
	if len(ports) == 0 {
		log.Fatal("No serial ports found!")
	}

	for _, port := range ports {
		if port.VID == *vid && port.PID == *pid && port.SerialNumber == *serialNumber {
			mode := &serial.Mode{
				BaudRate: 115200,
			}

			serialPort, err = serial.Open(port.Name, mode)
			if err != nil {
				log.Fatal(err)
			}
		}
	}

	hub := ws.NewHub()
	go hub.Run()

	go commander.Listen(hub, serialPort, serialPort)

	http.HandleFunc("/raspberry_pi_pico", func(w http.ResponseWriter, r *http.Request) {
		commander.ServeWS(hub, w, r, serialPort)
	})

	log.Printf("Server running on %s", Port)
	if err = http.ListenAndServe(Port, nil); err != nil {
		log.Fatal(err)
	}
}
