package main

import (
	"context"
	"errors"
	"fmt"
	"io"
	"log"
	"net"
	"strings"
	"time"
)

const serverAddr = "localhost:2137"

func main() {
	log.Println("Trying to connect to", serverAddr)
	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	log.Println("Operating for 5 seconds...")
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	defer cancel()

	go readLoop(ctx, conn)
	<-ctx.Done()
}

func bytesToHex(bytes []byte) string {
	b := strings.Builder{}
	for _, msgb := range bytes {
		b.WriteString(fmt.Sprintf("0x%02x", msgb))
	}
	return b.String()
}

func readLoop(ctx context.Context, conn net.Conn) {
	for {
		select {
		case <-ctx.Done():
			log.Println("Context done, exiting read loop")
			return

		default:
			bytes := make([]byte, 256)
			n, err := conn.Read(bytes)
			if err != nil && !errors.Is(err, io.EOF) {
				log.Fatal(err)
			}

			if n == 0 {
				continue
			}

			fmt.Printf("Received %d bytes: %s\n", n, bytesToHex(bytes))
		}
	}
}
