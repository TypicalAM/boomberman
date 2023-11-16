package main

import (
	"log"
	"time"
)

const serverAddr = "localhost:2137"

func main() {
	done := make([]chan struct{}, 3)
	log.Println("Creating a client")

	for i := 0; i < 3; i++ {
		if i == 2 {
			time.Sleep(4 * time.Second)
		}
		client := createClient()
		defer client.Close()
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
