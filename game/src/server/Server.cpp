#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include "Server.h"
#include <sys/epoll.h>
#include <csignal>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/trivial.hpp>
#include "../shared/msg/Builder.h"

void Server::handleClientMessage(Connection *conn, std::unique_ptr<GameMessage> msg) {
    switch (msg->type()) {
        case GET_ROOM_LIST: {
            // Create a room list and send it to the user
            std::lock_guard<std::mutex> lock(roomsMtx);
            std::vector<Builder::Room> room_list;
            std::vector<std::string> rooms_finished;
            for (const auto &pair: rooms) {
                if (pair.second->IsGameOver()) rooms_finished.push_back(pair.first);
                int player_count = pair.second->Players();
                if (player_count != MAX_PLAYERS)
                    room_list.push_back(Builder::Room{pair.first, player_count, MAX_PLAYERS});
            }

            for (const auto &key: rooms_finished) {
                LOG << "Destroyed a finished room: " << key;
                rooms.erase(key);
            }

            // Close the connection if we can't send the message
            if (!conn->Send(Builder::RoomList(room_list)).has_value()) {
                epoll_ctl(epollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
                shutdown(conn->sock, SHUT_RDWR);
                close(conn->sock);
            }
            return;
        }

        case JOIN_ROOM: {
            // Join a user to a room (if he specified the room) or create a new one
            auto room_msg = msg->joinroom();

            std::shared_ptr<Room> room;
            if (!room_msg.has_roomname()) {
                auto new_room_name = Util::RandomString(10);
                room = std::make_shared<Room>(createNamedLogger(new_room_name));
                std::thread(&Room::GameLoop, room).detach();
                std::lock_guard<std::mutex> lock(roomsMtx);
                rooms[new_room_name] = room;
            } else {
                if (rooms.find(room_msg.roomname()) == rooms.end()) {
                    // Close the connection if we can't send the message
                    LOG << "Client requested a room which doesn't exist: " << room_msg.roomname();
                    if (conn->Send(Builder::Error("There is no room with that name man")).has_value()) return;
                    epoll_ctl(epollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
                    shutdown(conn->sock, SHUT_RDWR);
                    close(conn->sock);
                    return;
                }

                std::lock_guard<std::mutex> lock(roomsMtx);
                room = rooms[room_msg.roomname()];
            }

            if (!room->CanJoin(room_msg.username())) {
                // Close the connection if we can't send the message
                if (conn->Send(Builder::Error("Cannot join this game")).has_value()) return;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
                shutdown(conn->sock, SHUT_RDWR);
                close(conn->sock);
                return;
            }

            LOG << "Joining player to game: " << room_msg.username();
            int idx = -1;
            for (int i = 0; i < conns.size(); i++) {
                if (conns[i]->sock == conn->sock) {
                    idx = i;
                }
            }

            if (!room->JoinPlayer(std::move(conns[idx]), room_msg.username())) {
                epoll_ctl(epollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
                shutdown(conn->sock, SHUT_RDWR);
                close(conn->sock);
            }

            conns.erase(conns.begin() + idx);
            return;
        }

        default: {
            // Close the connection if we can't send the message
            LOG << "Unexpected message type: " << msg->type();
            if (!conn->Send(Builder::Error("Unexpected message")).has_value()) {
                epoll_ctl(epollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
                shutdown(conn->sock, SHUT_RDWR);
                close(conn->sock);
            }
        }
    }
}

[[noreturn]] void Server::Run() {
    LOG << "Serving";

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
                conns.push_back(std::make_unique<Connection>(new_sock));
                epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.ptr = conns.back().get()}};
                if (epoll_ctl(epollSock, EPOLL_CTL_ADD, new_sock, &event) == -1) continue;
                LOG << "New connection accepted from: " << new_sock;
                continue;
            }

            // We got a message from a client
            auto conn = static_cast<Connection *>(events[i].data.ptr);
            auto msg = conn->Receive();
            if (!msg.has_value()) {
                LOG << "Closing connection since we can't receive data: " << conn->sock;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                shutdown(events[i].data.fd, SHUT_RDWR);
                close(events[i].data.fd);
                continue;
            }

            // Handle the message
            handleClientMessage(conn, std::move(msg.value()));
        }
    }
}

boost::log::sources::logger Server::createNamedLogger(const std::string &name) {
    boost::log::sources::logger room_logger;
    room_logger.add_attribute("Prefix", boost::log::attributes::constant<std::string>("[" + name + "] "));
    return room_logger;
}

Server::Server(int port) {
    logger = createNamedLogger("Server");
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

    LOG << "Listening on port: " << port;
}