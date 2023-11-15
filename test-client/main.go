package main

import (
	"errors"
	"io"
	"log"
	"net"

	"github.com/TypicalAM/boomberman/test-client/msg"
	"google.golang.org/protobuf/proto"
)

const serverAddr = "localhost:2137"

func main() {
	log.Println("Trying to connect to", serverAddr)
	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	msgs := make(chan *msg.GameMessage)
	go readLoop(conn, msgs)
	actLoop(msgs)
}

func readLoop(conn net.Conn, msgs chan *msg.GameMessage) {
	for {
		bytes := make([]byte, 512)
		n, err := conn.Read(bytes)
		if err != nil && !errors.Is(err, io.EOF) {
			log.Fatal(err)
		}

		if n == 0 {
			continue
		}

		var msg msg.GameMessage
		if err := proto.Unmarshal(bytes[:n], &msg); err != nil {
			log.Println("Failed to unmarshal message:", err)
			return
		}
		msgs <- &msg
	}
}

func actLoop(msgs chan *msg.GameMessage) {
	for gMsg := range msgs {
		log.Println("New message!")
		log.Println(gMsg.MessageType)
		if gMsg.MessageType == msg.MessageType_GAME_JOIN {
			gj := gMsg.GetGameJoin()
			log.Println("Game join:", gj.Name, gj.Color, gj.You)
		}
	}
}
