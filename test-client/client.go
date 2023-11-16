package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net"

	"github.com/TypicalAM/boomberman/test-client/pb"
	"google.golang.org/protobuf/proto"
)

type Client struct {
	conn net.Conn
	msgs chan *pb.GameMessage
	done chan struct{}

	username string
	entities []Entity
}

func New(serverAddr string) (*Client, error) {
	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		return nil, err
	}

	return &Client{
		conn:     conn,
		msgs:     make(chan *pb.GameMessage),
		done:     make(chan struct{}),
		entities: make([]Entity, 0),
	}, nil
}

func (c *Client) ActLoop() {
	for msg := range c.msgs {
		if c.username != "" {
			log.Printf("[%s] New message of type %s!\n", c.username, msg.MessageType)
		}

		switch msg.MessageType {
		case pb.MessageType_GAME_JOIN:
			gj := msg.GetGameJoin()
			if gj.You {
				// This is our player
				log.Printf("[%s] We joined the game\n", gj.Name)
				c.username = gj.Name
			} else {
				log.Printf("[%s] User %s has joined the game\n", c.username, gj.Name)
			}
			c.entities = append(c.entities, Player{gj.Name, Coord{0, 0}, gj.Color})

		case pb.MessageType_GAME_WAIT:
			gj := msg.GetGameWait()
			log.Printf("[%s] Waiting for %d players to join\n", c.username, gj.WaitingFor)

		case pb.MessageType_GOT_HIT:
			gh := msg.GetGotHit()
			log.Printf("[%s] Someone got hit: %s, they got %d lives left\n", c.username, gh.Name, gh.LivesRemaining)

		default:
			log.Printf("[%s] Unhandled type\n", c.username)
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
