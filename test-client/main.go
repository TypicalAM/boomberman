package main

import (
	"log"
	"time"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

const serverAddr = "localhost:2137"

func main() {
	done := make([]chan struct{}, 3)
	log.Println("Creating a client")

	for i := 0; i < 3; i++ {
		client := createClient()
		defer client.Close()
		done[i] = client.Done()
		if i != 2 {
			client.Send(&pb.GameMessage{
				MessageType: pb.MessageType_I_MOVE,
				Message:     &pb.GameMessage_IMove{IMove: &pb.IMoveMsg{X: 4, Y: 4}}, // Just out of range ;)
			})
		} else {
			time.Sleep(3 * time.Second)
			client.Send(&pb.GameMessage{
				MessageType: pb.MessageType_I_PLACE_BOMB,
				Message:     &pb.GameMessage_IPlaceBomb{IPlaceBomb: &pb.IPlaceBombMsg{X: 0, Y: 0}}, // Just out of range ;)
			})
		}
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
