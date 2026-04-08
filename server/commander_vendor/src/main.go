package main

import (
	commander "commander/src"
	"commander/src/ws"
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strconv"
	"syscall"
	"time"

	"github.com/google/gousb"
)

type endpoints struct {
	iface int
	in    int
	out   int
}

const (
	Port = ":80"
	Path = "commander_vendor"
)

func findBulkEndpoints(cfg *gousb.Config) (endpoints, error) {
	var ep endpoints
	var found bool
	for _, iface := range cfg.Desc.Interfaces {
		for _, alt := range iface.AltSettings {
			var inEp, outEp int
			for _, ep := range alt.Endpoints {
				switch ep.Direction {
				case gousb.EndpointDirectionOut:
					if ep.Address == 0x04 {
						// packet.MaxPacketOut = ep.MaxPacketSize
						outEp = ep.Number
					}
				case gousb.EndpointDirectionIn:
					if ep.Address == 0x84 {
						// packet.MaxPacketIn = ep.MaxPacketSize
						inEp = ep.Number
					}
				}
			}
			if inEp != 0 && outEp != 0 {
				ep = endpoints{iface: alt.Number, in: inEp, out: outEp}
				found = true
				break
			}
		}
		if found {
			break
		}
	}

	if !found {
		return ep, fmt.Errorf("no interface with bulk IN and OUT endpoints found")
	}
	// packet.MaxPacketsIn = packet.MaxPacketIn / packet.PacketSizeIn
	// packet.MaxPacketsOut = packet.MaxPacketOut / packet.PacketSizeIn
	return ep, nil
}

func main() {
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGTERM, syscall.SIGINT)

	go func() {
		<-sig
		os.Exit(0)
	}()

	ctx := gousb.NewContext()
	defer ctx.Close()

	vidString := flag.String("vid", "0", "USB vendor id")
	pidString := flag.String("pid", "0", "USB product id")
	flag.Parse()

	vid, err := strconv.ParseUint(*vidString, 0, 16)
	if err != nil {
		log.Fatalf("invalid vid: %v", err)
	}
	pid, err := strconv.ParseUint(*pidString, 0, 16)
	if err != nil {
		log.Fatalf("invalid pid: %v", err)
	}

	dev, err := ctx.OpenDeviceWithVIDPID(gousb.ID(vid), gousb.ID(pid))
	if err != nil {
		log.Fatalf("Could not open a device: %v", err)
	}
	defer dev.Close()

	cfg, err := dev.Config(1)
	if err != nil {
		log.Fatalf("config: %v", err)
	}
	defer cfg.Close()

	// dev.SetAutoDetach(true)

	ep, err := findBulkEndpoints(cfg)
	if err != nil {
		log.Fatal(err)
	}

	iface, err := cfg.Interface(ep.iface, 0)
	if err != nil {
		log.Fatalf("interface: %v", err)
	}
	defer iface.Close()

	epOut, err := iface.OutEndpoint(ep.out)
	if err != nil {
		log.Fatalf("%s.OutEndpoint(7): %v", iface, err)
	}

	epIn, err := iface.InEndpoint(ep.in)
	if err != nil {
		log.Fatalf("%s.InEndpoint: %v", iface, err)
	}

	// FLUSH OLD DATA
	c, cancel := context.WithTimeout(context.Background(), 100*time.Millisecond)
	defer cancel()
	buf := make([]byte, ws.RxSize)
	for {
		n, _ := epIn.ReadContext(c, buf)
		if n == 0 {
			break
		}
	}

	hub := ws.NewHub()
	go hub.Run()
	go commander.Listen(hub, epOut, epIn)

	http.HandleFunc("/", commander.ControlPanel)
	http.HandleFunc("/"+Path, func(w http.ResponseWriter, r *http.Request) {
		commander.ServeWS(hub, w, r, epOut)
	})

	log.Printf("Server running on:")
	log.Printf("  Local:   ws://localhost%s/%s", Port, Path)
	log.Printf("  Network: ws://%s%s/%s", commander.GetIP(), Port, Path)
	if err = http.ListenAndServe(Port, nil); err != nil {
		log.Fatal(err)
	}
}
