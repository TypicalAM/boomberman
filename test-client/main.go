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
	count := 3
	clients := make([]*Client, count)

	for i := 0; i < count; i++ {
		log.Println("Client", i, "connecting")
		random := rand.Intn(len(names))
		clients[i] = createClient(names[random])
		defer clients[i].Close()
		clients[i].Send(&pb.GameMessage{
			Type:    pb.MessageType_GET_ROOM_LIST,
			Message: &pb.GameMessage_GetRoomList{&pb.GetRoomList{}},
		})

		time.Sleep(200 * time.Millisecond)
	}

	leaveMsg := &pb.GameMessage{Type: pb.MessageType_I_LEAVE,
		Message: &pb.GameMessage_ILeave{&pb.ILeave{}}}
	clients[0].Send(leaveMsg)
	time.Sleep(200 * time.Millisecond)
	clients[1].Send(leaveMsg)

	time.Sleep(500 * time.Millisecond)

	for _, client := range clients {
		<-client.Done()
	}
}
