#include <thread>
#include <iostream>
#include "server/Server.h"
#include "client/Client.h"
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/trivial.hpp>

const int screenWidth = 800;
const int screenHeight = 450;

void init_logging() {
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "> %Prefix%%Message%");
}

int main(int argc, char *argv[]) {
    init_logging();
    BOOST_LOG_TRIVIAL(info) << "Starting boomberman";

    std::string target(argv[1]);

    if (target != "client" && target != "server") {
        std::cerr << "Expected one of: server, client" << std::endl;
        std::cerr << "Usage: " << argv[0] << " client [username]/server" << std::endl;
        return 1;
    }

    if (target == "client") {
        Client client(screenWidth, screenHeight);
        client.Run();
    } else {
        Server server(2137);
        std::thread(&Server::RunRoom, &server).detach();
        server.RunLobby();
    };

    return 0;
}