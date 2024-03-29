package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net"
	"strings"
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
			log.Printf("[%s] New message of type %s!\n", c.username, msg.Type)
		}

		switch msg.Type {
		case pb.MessageType_ROOM_LIST:
			// This is the list of the rooms, we can join the first free one
			rooms := msg.GetRoomList_().GetRooms()
			hasAvailableRoom := false
			if len(rooms) == 0 {
				c.Send(&pb.GameMessage{
					Type:    pb.MessageType_JOIN_ROOM,
					Message: &pb.GameMessage_JoinRoom{JoinRoom: &pb.JoinRoom{Username: c.username}},
				})
				log.Printf("[%s] Told the server to create a new room\n", c.username)
				continue
			}

			availableRoom := rooms[0]
			for _, room := range rooms {
				log.Printf("[%s] Room %s has %d players out of %d\n", c.username, room.Name, room.PlayerCount, room.MaxPlayerCount)
				if room.PlayerCount < room.MaxPlayerCount {
					hasAvailableRoom = true
					availableRoom = room
				}
			}

			if hasAvailableRoom {
				c.Send(&pb.GameMessage{
					Type:    pb.MessageType_JOIN_ROOM,
					Message: &pb.GameMessage_JoinRoom{JoinRoom: &pb.JoinRoom{Username: c.username, RoomName: &availableRoom.Name}},
				})
				log.Printf("[%s] Told the server to join room %s\n", c.username, availableRoom.Name)
			} else {
				c.Send(&pb.GameMessage{
					Type:    pb.MessageType_JOIN_ROOM,
					Message: &pb.GameMessage_JoinRoom{JoinRoom: &pb.JoinRoom{Username: c.username}},
				})
				log.Printf("[%s] Told the server to create a new room\n", c.username)
			}

		case pb.MessageType_OTHER_LEAVE:
			log.Printf("[%s] %s left the room\n", c.username, msg.GetOtherLeave().GetUsername())

		case pb.MessageType_WELCOME_TO_ROOM:
			log.Printf("[%s] Welcome to room!\n", c.username)
			var b strings.Builder
			for _, player := range msg.GetWelcomeToRoom().Players {
				b.WriteString(player.GetUsername())
				b.WriteString(" ")
			}
			log.Printf("[%s] Players in game: %s\n", c.username, b.String())

		case pb.MessageType_GAME_START:
			log.Printf("[%s] Game started!\n", c.username)
			c.isInGameMutex.Lock()
			c.isInGame = true
			c.isInGameMutex.Unlock()

		case pb.MessageType_GOT_HIT:
			log.Printf("[%s] Someone got hit: %s, lives remaning: %d\n", c.username, msg.GetGotHit().GetUsername(), msg.GetGotHit().GetLivesRemaining())

		case pb.MessageType_ERROR:
			log.Printf("[%s] Error: %s\n", c.username, msg.GetError().Error)
		}
	}
}

func (c *Client) ReadLoop() error {
	bytes := make([]byte, 512)
	for {
		bytesRead, err := c.conn.Read(bytes)
		if err != nil && !errors.Is(err, io.EOF) {
			log.Printf("[%s] Failed to read from connection: %s\n", c.username, err)
			return fmt.Errorf("failed to read from connection: %w", err)
		}

		if bytesRead == 0 {
			log.Printf("[%s] Connection has been closed gracefully \n", c.username)
			return fmt.Errorf("connection closed")

		}
		msgSize := int(bytes[0])
		if bytesRead == msgSize+1 {
			var msg pb.GameMessage
			if err := proto.Unmarshal(bytes[1:bytesRead], &msg); err != nil {
				log.Printf("[%s] Failed to unmarshall on exact: %v\n", c.username, err)
				return fmt.Errorf("failed to unmarshal message: %w", err)
			}

			c.msgs <- &msg
			continue
		}

		for bytesRead >= msgSize+1 {
			var msg pb.GameMessage
			if err := proto.Unmarshal(bytes[1:1+msgSize], &msg); err != nil {
				log.Printf("[%s] Failed to unmarshall on more: %v\n", c.username, err)
				return fmt.Errorf("failed to unmarshal message: %w", err)
			}

			c.msgs <- &msg
			bytesRead -= msgSize + 1
			bytes = bytes[:msgSize]
			msgSize = int(bytes[0])
		}

		if bytesRead == 0 {
			continue
		}

		readTotal := bytesRead
		for readTotal < msgSize+1 {
			bytesRead, err = c.conn.Read(bytes[readTotal:])
			if err != nil && !errors.Is(err, io.EOF) {
				log.Printf("[%s] Failed to read from connection: %s\n", c.username, err)
				return fmt.Errorf("failed to read from connection: %w", err)
			}

			if bytesRead == 0 {
				log.Printf("[%s] Connection has been closed gracefully \n", c.username)
				return fmt.Errorf("connection closed")
			}

			readTotal += bytesRead
		}

		var msg pb.GameMessage
		if err := proto.Unmarshal(bytes[1:1+msgSize], &msg); err != nil {
			log.Printf("[%s] Failed to unmarshall on additional: %v\n", c.username, err)
			return fmt.Errorf("failed to unmarshal message: %w", err)
		}

		c.msgs <- &msg
	}
}

func (c *Client) Send(msg *pb.GameMessage) error {
	dataRaw, err := proto.Marshal(msg)
	if err != nil {
		return fmt.Errorf("failed to marshal message: %w", err)
	}

	log.Printf("[%s] Sending message of type %s\n", c.username, msg.Type)
	data := append([]byte{uint8(len(dataRaw))}, dataRaw...)

	log.Printf("[%s] Has a length of %d\n", c.username, data[0])
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
