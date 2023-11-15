package main

import (
	"log"
)

const serverAddr = "localhost:2137"

func main() {
	log.Println("Creating a client")
	client, err := New(serverAddr)
	if err != nil {
		log.Fatal(err)
	}
	defer client.Close()

	log.Println("Starting the client loops")
	go client.ReadLoop()
	go client.ActLoop()
	<-client.Done()
}
