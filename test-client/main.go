package main

import (
	"log"
	"time"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

const serverAddr = "localhost:2137"

func main() {
	joinFirstGame()
}

func createClient(name string) *Client {
	client, err := New(serverAddr, name)
	if err != nil {
		log.Fatal(err)
	}

	log.Println("Starting the client loops")
	go client.ReadLoop()
	go client.GameLoop()
	return client
}

func joinFirstGame() {
	clients := make([]*Client, 3)
	names := []string{"Alice", "Bob", "Charlie"}
	log.Println("Creating a client")

	for i := 0; i < 3; i++ {
		clients[i] = createClient(names[i])
		defer clients[i].Close()
		clients[i].Send(&pb.GameMessage{
			MessageType: pb.MessageType_GET_ROOM_LIST,
			Message:     &pb.GameMessage_GetRoomList{&pb.GetRoomListMsg{}},
		})
		time.Sleep(1 * time.Second)
	}

	for _, client := range clients {
		<-client.Done()
	}
}
