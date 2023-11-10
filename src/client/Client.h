#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include <vector>

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

class Boomerman{
private:
    int player_x;
    int player_y;
    int health;
public:
    Boomerman(int player_x, int player_y, int health);
    int getBoomermanPosX();
    int getBoomermanPosY();
};

class Map{
private:
    int map[MAP_WIDTH][MAP_HEIGHT]{};
    int cols = int(sizeof(map)/ sizeof(map[-1]));
    int rows= int(sizeof(map[-1])/ sizeof(int));
    int size;
public:
    explicit Map(int size);

    void updateMap(const std::vector<std::array<int, 2>>& player_positions);
    void drawMap(Client *client);


};

#endif