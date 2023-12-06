#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include <string>
#include "../shared/game/Map.h"

class Client {
private:
    int width;
    int height;

public:
    Client(int width, int height);
    static void drawMap(Map* map);
    void Run(const char* player_name) const;
    int getDimension(const std::string& dimension) const;

    ~Client();
};
#endif