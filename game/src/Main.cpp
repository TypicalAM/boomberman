#include "client/Client.h"
#include "server/Server.h"
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <csignal>
#include <functional>
#include <iostream>

const int screenWidth = 800;
const int screenHeight = 450;

void init_logging() {
  boost::log::add_common_attributes();
  boost::log::add_console_log(std::cout, boost::log::keywords::format =
                                             "> %Prefix%%Message%");
}

void shutdownServer(Server *srv, int signal) {
  std::cout << "helo" << std::endl;
  BOOST_LOG_TRIVIAL(info) << "SIGINT received, exiting gracefully" << std::endl;
  srv->Shutdown();
  exit(0);
}

int main(int argc, char *argv[]) {
  init_logging();
  BOOST_LOG_TRIVIAL(info) << "Starting boomberman";

  std::string target(argv[1]);

  if (target != "client" && target != "server") {
    std::cerr << "Expected one of: server, client" << std::endl;
    std::cerr << "Usage: " << argv[0] << " client [username]/server"
              << std::endl;
    return 1;
  }

  if (target == "client") {
    Client client(screenWidth, screenHeight);
    client.Run();
  } else {
    Server server(2137);
    std::function<void(int)> handler =
        std::bind(shutdownServer, &server, std::placeholders::_1);
    std::signal(SIGINT,
                static_cast<void (*)(int)>(
                    handler.target<void(int)>())); // yay! std::bind
    server.Run();
  };

  return 0;
}
