package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net"
	"sync"

	"github.com/TypicalAM/boomberman/test-client/pb"
	"google.golang.org/protobuf/proto"
)

type Client struct {
	conn net.Conn
	msgs chan *pb.GameMessage
	done chan struct{}

	isInGame      bool
	isInGameMutex sync.Mutex

	username string
	entities []Entity
}

func New(serverAddr string, name string) (*Client, error) {
	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		return nil, err
	}

	return &Client{
		conn:     conn,
		msgs:     make(chan *pb.GameMessage),
		done:     make(chan struct{}),
		entities: make([]Entity, 0),
		username: name,
	}, nil
}

func (c *Client) GameLoop() {
	for msg := range c.msgs {
		if c.username != "" {
			log.Printf("[%s] New message of type %s!\n", c.username, msg.MessageType)
		}

		switch msg.MessageType {
		case pb.MessageType_ROOM_LIST:
			// If the first room is empty join new room
			rooms := msg.GetRoomList_().GetRooms()
			hasAvailableRoom := false
			if len(rooms) == 0 {
				c.Send(&pb.GameMessage{
					MessageType: pb.MessageType_JOIN_ROOM,
					Message:     &pb.GameMessage_JoinRoom{JoinRoom: &pb.JoinRoomMsg{Username: c.username}},
				})
				log.Println("Told the server to create a new room")
				continue
			}

			availableRoom := rooms[0]
			for _, room := range rooms {
				log.Printf("[%s] Room %s has %d players\n", c.username, room.Name, room.Players)
				if room.Players < room.MaxPlayers {
					hasAvailableRoom = true
				}
			}

			if hasAvailableRoom {
				c.Send(&pb.GameMessage{
					MessageType: pb.MessageType_JOIN_ROOM,
					Message:     &pb.GameMessage_JoinRoom{JoinRoom: &pb.JoinRoomMsg{Username: c.username, Room: availableRoom}},
				})
				log.Printf("[%s] Told the server to join room %s\n", c.username, availableRoom.Name)
			}

		case pb.MessageType_ERROR:
			log.Printf("[%s] Error: %s\n", c.username, msg.GetError().Error)
		}
	}
}

func (c *Client) ReadLoop() error {
	bytes := make([]byte, 512)
	for {
		n, err := c.conn.Read(bytes)
		if err != nil && !errors.Is(err, io.EOF) {
			log.Fatal(err)
		}

		if n == 0 {
			continue
		}

		var msg pb.GameMessage
		if err := proto.Unmarshal(bytes[:n], &msg); err != nil {
			return fmt.Errorf("failed to unmarshal message: %w", err)
		}
		c.msgs <- &msg
	}
}

func (c *Client) Send(msg *pb.GameMessage) error {
	data, err := proto.Marshal(msg)
	if err != nil {
		return fmt.Errorf("failed to marshal message: %w", err)
	}

	n, err := c.conn.Write(data)
	if err != nil || n != len(data) {
		return fmt.Errorf("failed to send message: %w", err)
	}

	return nil
}

func (c *Client) Close() error {
	return c.conn.Close()
}

func (c *Client) Done() chan struct{} {
	return c.done
}
