#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include <vector>
#include <array>

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

class Bomb{
public:
    int pos_x, pos_y;
    int explosion, size;
    double plant_time, animation_start, ttl;
    int state;
    bool should_explode=false;
    Bomb(int pos_x, int pos_y, int explosion, int size, float ttl);
    void animateOrBoom();
};

class Map{
private:
    int map[MAP_WIDTH][MAP_HEIGHT]{};
    int cols = int(sizeof(map)/ sizeof(map[-1]));
    int rows= int(sizeof(map[-1])/ sizeof(int));
    int size;
    std::vector<Bomb> bombs;
public:
    explicit Map(int size);

    int getSquareState(int x, int y);
    void setSquareState(int x, int y, int state);

    void localMapUpdate(const int *pos) ;
    void drawMap(Client *client);
    void addLocalBomb(int* pos);
    void drawLocalBombs(int offset, float start_x, float start_y);
};


class Boomerman{
private:
    int* position;
    int health;
public:
    Boomerman(int start_x, int start_y, int health);
    [[nodiscard]] int* getBoomermanPos() const;
    void setBoomermanPos(int new_x, int new_y);
    void move(Map *map, int* current_position, int x, int y);
    void shitYourself(Map *map);

};
#endif