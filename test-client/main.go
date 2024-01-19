package main

import (
	"bufio"
	"log"
	"math/rand"
	"os"
)

const serverAddr = "localhost:2137"

func main() {
	runScenario(1000, loadTest)
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

func runScenario(count int, scenario func([]Client)) {
	clients := make([]Client, count)
	names := loadNames()

	for i := 0; i < count; i++ {
		log.Println("Starting client: ", i)
		clients[i] = *createClient(names[rand.Intn(len(names))])
		defer clients[i].Close()
	}

	scenario(clients)

	for i := range clients {
		<-clients[i].Done()
	}
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
