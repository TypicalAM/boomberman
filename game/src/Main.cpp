#include "client/Client.h"
#include "server/Server.h"
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <csignal>
#include <iostream>

const int screenWidth = 800;
const int screenHeight = 450;

void init_logging() {
  boost::log::add_common_attributes();
  boost::log::add_console_log(std::cout, boost::log::keywords::format =
                                             "> %Prefix%%Message%");
}

int main(int argc, char *argv[]) {
  init_logging();
  signal(SIGPIPE, SIG_IGN);
  BOOST_LOG_TRIVIAL(info) << "Starting boomberman";

  if (argc < 2) {
    std::cerr << "Expected one of: server, client" << std::endl;
    std::cerr << "Usage: " << argv[0] << " client [username]/server"
              << std::endl;
    return 1;
  }

  std::string target(argv[1]);
  if (target != "client" && target != "server") {
    std::cerr << "Expected one of: server, client" << std::endl;
    std::cerr << "Usage: " << argv[0] << " client/server" << std::endl;
    return 1;
  }

  bool is_server = target == "server";
  if (is_server) {
    if (argc < 3) {
      std::cerr << "Expected port" << std::endl;
      std::cerr << "Usage: " << argv[0] << " server 1234" << std::endl;
      return 1;
    }

    int port;
    try {
      port = std::stoi(argv[2]);
    } catch (std::exception const &e) {
      std::cerr << "Expected a port number" << std::endl;
      std::cerr << "Usage: " << argv[0] << " server 1234" << std::endl;
      return 1;
    }

    if (port > 65535 || port < 1000) {
      std::cerr << "Expected a valid port bumber" << std::endl;
      std::cerr << "Usage: " << argv[0] << " server 1234" << std::endl;
      return 1;
    }

    Server server(port);
    server.Run();
    server.Cleanup();
  } else {
    if (argc < 4) {
      std::cerr << "Expected a server address and a port" << std::endl;
      std::cerr << "Usage: " << argv[0] << " client 127.0.0.1 1234"
                << std::endl;
      return 1;
    }

    int port;
    try {
      port = std::stoi(argv[2]);
    } catch (std::exception const &e) {
      std::cerr << "Expected a valid port bumber" << std::endl;
      std::cerr << "Usage: " << argv[0] << " client 127.0.0.1 1234"
                << std::endl;
      return 1;
    };

    Client client(screenWidth, screenHeight);
    client.Run(argv[2], port);
    return 0;
  }
}
