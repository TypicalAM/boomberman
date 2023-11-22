#include <thread>
#include <iostream>
#include "server/Server.h"
#include "client/Client.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

const int screenWidth = 800;
const int screenHeight = 450;

void init_logging() {
    boost::log::add_common_attributes();
}

int main(int argc, char *argv[]) {
    init_logging();
    BOOST_LOG_TRIVIAL(info) << "Hello bombaman";

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " client/server" << std::endl;
        return 1;
    }

    std::string target(argv[1]);

    if (target != "client" && target != "server") {
        std::cerr << "Expected one of: server, client" << std::endl;
        return 1;
    }

    if (target == "client") {
        Client client(screenWidth, screenHeight);
        client.Run();
    } else {
        Server server(2137);
        server.Run();
    }

    return 0;
}