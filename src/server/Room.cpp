#include <iostream>
#include <utility>
#include <thread>
#include "Room.h"
#include "ClientHandler.h"
#include "../shared/messages/server/GameJoin.h"

void Room::GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        msgQueueMtx->lock();
        if (msgQueue->empty()) {
            msgQueueMtx->unlock();
            continue;
        }

        auto msg = std::move(msgQueue->front()); // Acquire ownership of queue front
        msgQueue->pop();

        std::cout << "Cool message in gameloop: " << msg->name() << std::endl;
        auto author = msg->getAuthor();
        if (!author.has_value())
            throw std::runtime_error("no author in queue message");

        int idx = -1;
        for (int j = 0; j < handlers.size(); j++) {
            if (handlers[j]->GetClient() == author.value()) {
                idx = j;
                break;
            }
        }

        if (idx == -1)
            throw std::runtime_error("author not found in message");

        std::cout << "The author of the message is " << idx << std::endl;

        msgQueueMtx->unlock();
    }
}

bool Room::CanJoin() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    return MAX_PLAYERS - clientCount > 0;
}

void Room::JoinPlayer(int fd) {
    std::lock_guard<std::mutex> lock(handlerMtx);
    auto handler = std::make_shared<ClientHandler>(fd, msgQueue, msgQueueMtx);
    std::thread(&ClientHandler::ReadLoop, handler).detach();
    handlers.push_back(handler);
    clientCount++;

    auto msg = GameJoin(Color::RED);
    handler->Write(&msg); // NOTE: We need to take a stack pointer because of object slicing
    std::cout << "Started a new read thread for user " << fd << std::endl;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    msgQueue = std::make_shared<std::queue<std::unique_ptr<Message>>>();
    msgQueueMtx = std::make_shared<std::mutex>();
}