package main

import (
	"bufio"
	"log"
	"math/rand"
	"os"
	"time"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

const serverAddr = "localhost:2137"

func main() {
	names := loadNames()
	joinFirstGame(names)
}

func loadNames() []string {
	file, err := os.Open("names.txt")
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	names := make([]string, 0)
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		names = append(names, scanner.Text())
	}

	return names
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

func joinFirstGame(names []string) {
	log.Println("Creating a client")
	count := 4
	clients := make([]*Client, count)

	for i := 0; i < count; i++ {
		log.Println("Client", i, "connecting")
		random := rand.Intn(len(names))
		clients[i] = createClient(names[random])
		defer clients[i].Close()
		clients[i].Send(&pb.GameMessage{
			MessageType: pb.MessageType_GET_ROOM_LIST,
			Message:     &pb.GameMessage_GetRoomList{&pb.GetRoomListMsg{}},
		})

		time.Sleep(200 * time.Millisecond)
	}

	_ = &pb.GameMessage{MessageType: pb.MessageType_I_MOVE,
		Message: &pb.GameMessage_IMove{&pb.IMoveMsg{X: 5.0, Y: 5.0}}}

	placeBombMsg := &pb.GameMessage{MessageType: pb.MessageType_I_PLACE_BOMB,
		Message: &pb.GameMessage_IPlaceBomb{&pb.IPlaceBombMsg{X: 0.0, Y: 0.0}}}
	log.Println("Placing a bomb")
	clients[3].Send(placeBombMsg)
	time.Sleep(1 * time.Second)
	log.Println("Placing a bomb")
	clients[3].Send(placeBombMsg)
	time.Sleep(1 * time.Second)
	log.Println("Placing a bomb")
	clients[3].Send(placeBombMsg)
	time.Sleep(1 * time.Second)
	log.Println("Client 0 should win the game")

	time.Sleep(1 * time.Second)

	log.Println("Waiting for the game to end...")
	time.Sleep(7 * time.Second)
	random := rand.Intn(len(names))
	clients[count-1] = createClient(names[random])
	defer clients[count-1].Close()
	clients[count-1].Send(&pb.GameMessage{
		MessageType: pb.MessageType_GET_ROOM_LIST,
		Message:     &pb.GameMessage_GetRoomList{&pb.GetRoomListMsg{}},
	})

	for _, client := range clients {
		<-client.Done()
	}
}
