#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <random>
#include <thread>
#include "Server.h"
#include <sys/epoll.h>
#include <csignal>
#include "../shared/Util.h"
#include "../shared/Channel.h"
#include "../shared/Builder.h"

void Server::handleClientMessage(int sock, std::unique_ptr<GameMessage> msg) {
    switch (msg->message_type()) {
        case GET_ROOM_LIST: {
            // Create a room list and send it to the user
            std::lock_guard<std::mutex> lock(roomsMtx);
            std::vector<Builder::Room> room_list;
            for (const auto &pair: rooms) {
                int player_count = pair.second->Players();
                if (player_count != MAX_PLAYERS)
                    room_list.push_back(Builder::Room{pair.first, player_count, MAX_PLAYERS});
            }

            // Close the connection if we can't send the message
            if (!Channel::Send(sock, Builder::RoomList(room_list)).has_value()) {
                epoll_ctl(epollSock, EPOLL_CTL_DEL, sock, nullptr);
                close(sock);
            }
            return;
        }

        case JOIN_ROOM: {
            // Join a user to a room (if he specified the room) or create a new one
            auto room_msg = msg->join_room();

            std::shared_ptr<Room> room;
            if (!room_msg.has_room()) {
                auto new_room_name = Util::RandomString(10);
                room = std::make_shared<Room>(new_room_name);
                std::thread(&Room::GameLoop, room).detach();
                std::thread(&Room::ReadLoop, room).detach();
                std::lock_guard<std::mutex> lock(roomsMtx);
                rooms[new_room_name] = room;
            } else {
                if (rooms.find(room_msg.room().name()) == rooms.end()) {
                    // Close the connection if we can't send the message
                    if (Channel::Send(sock, Builder::Error("There is no room with that name man")).has_value()) return;
                    epoll_ctl(epollSock, EPOLL_CTL_DEL, sock, nullptr);
                    close(sock);
                    return;
                }

                std::lock_guard<std::mutex> lock(roomsMtx);
                room = rooms[room_msg.room().name()];
            }

            if (!room->CanJoin(room_msg.username())) {
                // Close the connection if we can't send the message
                if (Channel::Send(sock, Builder::Error("Cannot join this game")).has_value()) return;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, sock, nullptr);
                close(sock);
                return;
            }

            std::cout << "Joining player to game: " << room_msg.username() << std::endl;
            epoll_ctl(epollSock, EPOLL_CTL_DEL, sock, nullptr);
            if (!room->JoinPlayer(sock, room_msg.username())) close(sock);
            return;
        }

        default: {
            // Close the connection if we can't send the message
            std::cout << "Unexpected message type: " << msg->message_type() << std::endl;
            if (!Channel::Send(sock, Builder::Error("Unexpected message")).has_value()) {
                epoll_ctl(epollSock, EPOLL_CTL_DEL, sock, nullptr);
                close(sock);
            }
        }
    }
}

[[noreturn]] void Server::Run() {
    std::cout << "Serving" << std::endl;

    epoll_event events[10];
    while (true) {
        int event_num = epoll_wait(epollSock, events, 10, -1);
        for (int i = 0; i < event_num; ++i) {
            // If this isn't an in event, ignore it
            if (!(events[i].events & EPOLLIN)) continue;

            // Check if server event
            if (events[i].data.fd == srvSock) {
                int new_sock = accept(srvSock, nullptr, nullptr);
                if (new_sock == -1) continue;
                epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.fd = new_sock}};
                if (epoll_ctl(epollSock, EPOLL_CTL_ADD, new_sock, &event) == -1) continue;
                std::cout << "New connection accepted from: " << new_sock << std::endl;
                continue;
            }

            // We got a message from a client
            auto msg = Channel::Receive(events[i].data.fd);
            if (!msg.has_value()) {
                std::cout << "Closing connection since we can't receive data: " << events[i].data.fd << std::endl;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                close(events[i].data.fd);
                continue;
            }

            // Handle the message
            handleClientMessage(events[i].data.fd, std::move(msg.value()));
        }
    }
}

Server::Server(int port) {
    sockaddr_in localAddress{AF_INET, htons(port), htonl(INADDR_ANY)};
    int one = 1;
    // Socket creation
    if ((srvSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("socket creation failed");
    }

    // Set socket options
    if (setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        close(srvSock); // Clean up on failure
        throw std::runtime_error("setting socket options failed");
    }

    // Bind the socket
    if (bind(srvSock, (sockaddr *) &localAddress, sizeof(localAddress)) == -1) {
        close(srvSock); // Clean up on failure
        throw std::runtime_error("bind failed");
    }

    // Listen for incoming connections
    if (listen(srvSock, 1) == -1) {
        close(srvSock);
        throw std::runtime_error("listen failed");
    }

    // Create epoll instance
    if ((epollSock = epoll_create1(0)) == -1) {
        close(epollSock); // Clean up on failure
        throw std::runtime_error("epoll creation failed");
    }

    // Add server socket to epoll
    epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.fd = srvSock}};
    if (epoll_ctl(epollSock, EPOLL_CTL_ADD, srvSock, &event) == -1) {
        close(srvSock); // Clean up on failure
        close(epollSock); // Clean up on failure
        throw std::runtime_error("epoll control failed");
    }

    std::cout << "Listening on port 2137" << std::endl;
}