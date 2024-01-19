package main

import (
	"time"

	"github.com/TypicalAM/boomberman/test-client/pb"
)

func getIntoRoom(clients []Client) {
	getRoomList := &pb.GameMessage{Type: pb.MessageType_GET_ROOM_LIST}
	for i := range clients {
		clients[i].Send(getRoomList)
		time.Sleep(20 * time.Millisecond)
	}
}

func loadTest(clients []Client) {
	getIntoRoom(clients)

	placeBomb := &pb.GameMessage{
		Type:    pb.MessageType_I_PLACE_BOMB,
		Message: &pb.GameMessage_IPlaceBomb{IPlaceBomb: &pb.IPlaceBomb{X: 1, Y: 1}},
	}

	time.Sleep(1 * time.Second)

	for i := range clients {
		clients[i].Send(placeBomb)
		time.Sleep(50 * time.Millisecond)
	}

	time.Sleep(3 * time.Second)
	clients = append(clients, *createClient("TEST"))
	getRoomList := &pb.GameMessage{Type: pb.MessageType_GET_ROOM_LIST}
	clients[len(clients)-1].Send(getRoomList)
}
