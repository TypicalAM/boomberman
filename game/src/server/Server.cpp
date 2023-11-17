#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <random>
#include <thread>
#include <fcntl.h>
#include "Server.h"
#include "../shared/Util.h"
#include "../shared/Channel.h"
#include "../shared/Builder.h"

std::optional<int> Server::setup() const {
    sockaddr_in localAddress{AF_INET, htons(this->port), htonl(INADDR_ANY)};
    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(servSock, (sockaddr *) &localAddress, sizeof(localAddress))) return std::nullopt;
    listen(servSock, 1);
    return servSock;
}

void Server::acceptConnections() {
    std::cout << "Serving on port: " << this->port << std::endl;
    while (true) {
        int clientSock = accept(sock, nullptr, nullptr);
        if (clientSock == -1) throw std::runtime_error("cannot accept connections");
        if (fcntl(clientSock, F_SETFL, O_NONBLOCK) == -1) throw std::runtime_error("cannot set non-blocking mode");
        std::lock_guard<std::mutex> lock(roomsMtx);
        lobbySockets.push_back(clientSock);
    }
};

void Server::handleLobbyClients() {
    std::vector<int> to_remove;
    for (const auto &lobbySock: lobbySockets) {
        auto msg = Channel::Receive(lobbySock);
        if (!msg.has_value()) continue;
        switch (msg.value()->message_type()) {
            case GET_ROOM_LIST: {
                std::vector<Builder::Room> room_list;
                roomsMtx.lock();
                for (const auto &pair: rooms)
                    room_list.push_back(Builder::Room{
                            .name = pair.first,
                            .players = pair.second->Players(),
                            .maxPlayers = MAX_PLAYERS,
                    });
                roomsMtx.unlock();
                Channel::Send(lobbySock, Builder::RoomList(room_list));
                break;
            }

            case JOIN_ROOM: {
                std::lock_guard<std::mutex> lock(roomsMtx);
                auto room_msg = msg.value()->join_room();

                std::shared_ptr<Room> room;
                if (!room_msg.has_room()) {
                    auto new_room_name = Util::RandomString(10);
                    room = std::make_shared<Room>(new_room_name);
                    std::thread(&Room::GameLoop, room).detach();
                    rooms[new_room_name] = room;
                } else {
                    if (rooms.find(room_msg.room().name()) == rooms.end()) {
                        Channel::Send(lobbySock, Builder::Error("There is no room with that name man"));
                        return;
                    }

                    room = rooms[room_msg.room().name()];
                }

                if (!room->CanJoin(room_msg.username())) {
                    Channel::Send(lobbySock, Builder::Error("Cannot join this game"));
                    return;
                }

                std::cout << "Joining player to game: " << room_msg.username() << std::endl;
                room->JoinPlayer(lobbySock, room_msg.username());
                to_remove.push_back(lobbySock);
                break;
            }

            default:
                std::cout << "Unexpected message type: " << msg.value()->message_type() << std::endl;
                Channel::Send(lobbySock, Builder::Error("Unexpected message"));
        };
    }

    for (const auto &index: to_remove)
        lobbySockets.erase(lobbySockets.begin() + index);
}

[[noreturn]] void Server::Run() {
    std::optional<int> serv = setup();
    if (!serv.has_value())
        throw std::runtime_error("failed to set up the tcp server");
    this->sock = serv.value();

    // Loop indefinitely
    std::thread(&Server::acceptConnections, this).detach();
    while (true) handleLobbyClients();
}

Server::Server(int port) {
    this->port = port;
}