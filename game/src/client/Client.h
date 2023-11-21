#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include <string>

#define MAP_WIDTH 18
#define MAP_HEIGHT 10

class Client {
private:
    int width;
    int height;

public:
    Client(int width, int height);

    void Run();
    int getDimension(const std::string& dimension) const;

    ~Client();
};
#endif