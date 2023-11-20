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

		time.Sleep(200 * time.Millisecond)
	}

	time.Sleep(1 * time.Second)

	moveAway := &pb.GameMessage{MessageType: pb.MessageType_I_MOVE,
		Message: &pb.GameMessage_IMove{&pb.IMoveMsg{X: 5, Y: 5}}}
	clients[0].Send(moveAway)
	clients[1].Send(moveAway)

	time.Sleep(200 * time.Millisecond)

	placeBomb := &pb.GameMessage{MessageType: pb.MessageType_I_PLACE_BOMB,
		Message: &pb.GameMessage_IPlaceBomb{&pb.IPlaceBombMsg{X: 0, Y: 0}}}
	clients[2].Send(placeBomb)
	time.Sleep(500 * time.Millisecond)
	clients[2].Send(placeBomb)
	time.Sleep(500 * time.Millisecond)
	clients[2].Send(placeBomb)

	// Then let's try to move, we should be blocked
	time.Sleep(5 * time.Second)
	clients[2].Send(moveAway)

	for _, client := range clients {
		<-client.Done()
	}
}
