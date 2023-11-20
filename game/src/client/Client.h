#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include <vector>
#include <array>
#include <string>

#define MAP_WIDTH 18
#define MAP_HEIGHT 10

;

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
    std::vector<Bomb> bombs;
public:
    int offset{}, start_x{}, start_y{};
    int size;
    explicit Map(Client* client, int size);
    int* getColsNrows();
    int getSquareState(int x, int y);
    void setSquareState(int x, int y, int state);
    void drawMap(Client *client);
    void debug();
};

class Boomerman{
private:
    int* position;
    int health;
public:
    int id;
    Boomerman(int id, int start_x, int start_y, int health);
    [[nodiscard]] int* getBoomermanPos() const;
    void setBoomermanPos(int new_x, int new_y);
    void drawPlayer(int x, int y, int size);
    void move(Map *map, int* current_position, int x, int y);
    void shitYourself(Map *map);
};

class Wall{
private:
    int pos_x, pos_y;
    bool is_destructible;
public:
    Wall(int pos_x, int pos_y, bool is_descructible);
    int* getPosition();
    int isDescructible();
    void drawWall(int x, int y, int size);
};

class EntityHandler{
public:
    std::vector<Boomerman> players;
    std::vector<Bomb> bombs;
    std::vector<Wall> walls;

    void placeWalls(Map* map);
    int destroyWall(int x, int y);

    void drawWalls(Map* map);
    void drawPlayers(Map* map);
};


#endif