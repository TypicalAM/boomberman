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

class Map{
private:
    int map[MAP_WIDTH][MAP_HEIGHT]{};
    int cols = int(sizeof(map)/ sizeof(map[-1]));
    int rows= int(sizeof(map[-1])/ sizeof(int));
    int size;
public:
    explicit Map(int size);

    int getSquareState(int x, int y);
    void setSquareState(int x, int y, int state);

    void localMapUpdate(const int *pos) ;
    void drawMap(Client *client);
};

class Boomerman{
private:
    int player_x;
    int player_y;
    int health;
public:
    Boomerman(int player_x, int player_y, int health);
    int* getBoomermanPos();
    void setBoomermanPos( int x, int y);
    void move(Map *map, int* current_position, int x, int y);
};

#endif