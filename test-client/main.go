package main

import (
	"log"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

const serverAddr = "localhost:2137"

func main() {
	done := make([]chan struct{}, 3)
	log.Println("Creating a client")

	for i := 0; i < 3; i++ {
		client := createClient()
		defer client.Close()
		if i == 2 {
			// Just to demo for now
			client.Send(&pb.GameMessage{
				MessageType: pb.MessageType_I_MOVE,
				Message:     &pb.GameMessage_IMove{IMove: &pb.IMoveMsg{X: 1, Y: 1}},
			})
		}
		done[i] = client.Done()
	}

	for _, d := range done {
		<-d
	}
}

func createClient() *Client {
	client, err := New(serverAddr)
	if err != nil {
		log.Fatal(err)
	}

	log.Println("Starting the client loops")
	go client.ReadLoop()
	go client.ActLoop()
	return client
}
