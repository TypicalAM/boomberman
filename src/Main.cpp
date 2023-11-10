#include <thread>
#include <iostream>
#include "client/Client.h"
#include "server/Sever.h"

const int screenWidth = 800;
const int screenHeight = 450;

int main(int argc, char *argv[]) {
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