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

  if (argc != 2) {
    std::cerr << "Expected one of: server, client" << std::endl;
    std::cerr << "Usage: " << argv[0] << " client [username]/server"
              << std::endl;
    return 1;
  }

  std::string target(argv[1]);
  if (target != "client" && target != "server") {
    std::cerr << "Expected one of: server, client" << std::endl;
    std::cerr << "Usage: " << argv[0] << " client [username]/server"
              << std::endl;
    return 1;
  }

  if (target == "server") {
    Server server(2137);
    server.Run();
    server.Cleanup();
  } else {
    Client client(screenWidth, screenHeight);
    client.Run();
  };

  return 0;
}
