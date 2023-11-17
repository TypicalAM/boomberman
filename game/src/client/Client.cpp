#include <iostream>
#include <cstring>
#include <algorithm>
#include "Client.h"
#include "raylib.h"

void Client::Run() {
    Map map(25);
    Boomerman local_boomberman(1,1,3);
    int* current_local_Boomerman_pos;

    while (!WindowShouldClose()) {
        current_local_Boomerman_pos = local_boomberman.getBoomermanPos();
        if(IsKeyPressed(KEY_SPACE)){
            local_boomberman.shitYourself(&map);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 0, 1);
        }

        map.localMapUpdate(local_boomberman.getBoomermanPos());

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        map.drawMap(this);
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
}

Map::Map(int size){
    this->size = size;
    memset( this->map, 0, sizeof(this->map) );

    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(x==0 || x==this->cols-1 || y==0 || y==this->rows-1) this->map[x][y]=1;
        }
    }
}
int Map::getSquareState(int x, int y) {
    return this->map[x][y];
}
void Map::setSquareState(int x, int y, int state) {
    this->map[x][y]=state;
}
void Map::localMapUpdate(const int *pos) {
    this->map[pos[0]][pos[1]]=2;
    for(auto& bomb: this->bombs){
        if(bomb.should_explode){
            for(int horizontal=0; horizontal<bomb.explosion; horizontal++){
                for(int vertical=0; vertical<bomb.explosion; vertical++){
                    if(this->map[bomb.pos_x+horizontal][bomb.pos_y+vertical]!=1){
                        if(this->map[bomb.pos_x+horizontal][bomb.pos_y]!=1) this->map[bomb.pos_x+horizontal][bomb.pos_y]=3;
                        if(this->map[bomb.pos_x-horizontal][bomb.pos_y]!=1) this->map[bomb.pos_x-horizontal][bomb.pos_y]=3;
                        if(this->map[bomb.pos_x][bomb.pos_y+vertical]!=1) this->map[bomb.pos_x][bomb.pos_y+vertical]=3;
                        if(this->map[bomb.pos_x][bomb.pos_y-vertical]!=1) this->map[bomb.pos_x][bomb.pos_y-vertical]=3;
                    }
                }
            };
        }
    }
}
void Map::drawMap(Client *client) {
    int offset = int((this->size*4)/3);
    int start_x = int(client->getDimension("width")/2 - (cols/2)*offset);
    int start_y = int(client->getDimension("height")/2 - (rows/2)*offset);

    auto color = GRAY;
    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(this->map[x][y]==1) color = BLACK; // WALL
            else if(this->map[x][y]==2 || this->map[x][y]==5) color = BLUE; // PLAYER
            else if(this->map[x][y]==3) color = RED; // EXPLOSION
            else color = GRAY; // EMPTY SQUARE
            DrawRectangle(x * offset + start_x, y * offset + start_y, this->size, this->size, color);
            const char *info=std::to_string(this->map[x][y]).c_str();
            DrawText(info,x * offset + start_x, y * offset + start_y,10,YELLOW);
        }
    }
    this->drawLocalBombs(offset, start_x+this->size/2, start_y+this->size/2);
}
void Map::addLocalBomb(int* pos) {
    Bomb newBomb(pos[0],pos[1],3,15,3.0);
    this->bombs.push_back(newBomb);
}

void Map::drawLocalBombs(int offset, float start_x, float start_y) {
    for(auto& bomb: this->bombs){
        bomb.animateOrBoom();
        Color animation_color;
        if(bomb.state==-1) animation_color=BLACK;
        else animation_color=RED;
        DrawRectangle(bomb.pos_x*offset+start_x-bomb.size/2, bomb.pos_y*offset+start_y-bomb.size/2,bomb.size,bomb.size,animation_color);
    }
}

Boomerman::Boomerman(int start_x, int start_y, int health) {
    this->position=new int[2];
    this->position[0]=start_x;
    this->position[1]=start_y;
    this->health=health;
    this->id=0;
}
int* Boomerman::getBoomermanPos() const{
    int* pos = new int[2];
    pos[0]=this->position[0];
    pos[1]=this->position[1];
    return pos;
}
void Boomerman::setBoomermanPos(int new_x, int new_y) {
    this->position[0]=new_x;
    this->position[1]=new_y;
}
void Boomerman::move(Map* map, int* curr_pos, int x, int y) {
    int new_x = curr_pos[0]+x;
    int new_y = curr_pos[1]+y;
    int is_wall = map->getSquareState(new_x,new_y);
    if(is_wall!=1){
        this->setBoomermanPos(new_x, new_y);
        map->setSquareState(curr_pos[0],curr_pos[1],0);
    }
}

void Boomerman::shitYourself(Map *map) {
    map->addLocalBomb(this->position);
}

Client::Client(int width, int height) {
    this->width = width;
    this->height = height;

    InitWindow(width, height, "Boomberman client");
    SetTargetFPS(60);
}
int Client::getDimension(const std::string& dimension) const {
    if(dimension=="height") return this->height;
    else if(dimension=="width") return this->width;
    else return -1;
}
Client::~Client() {
    std::cout << "Closing window..." << std::endl;
    CloseWindow();
}

Bomb::Bomb(int pos_x, int pos_y, int explosion, int size, float ttl) {
    this->pos_x=pos_x;
    this->pos_y=pos_y;
    this->explosion=explosion;
    this->size=size;
    this->ttl=ttl;
    this->animation_start=GetTime();
    this->plant_time=this->animation_start;
    this->state=-1;
}

void Bomb::animateOrBoom() {
    double now=GetTime();
    if(now-this->animation_start>=0.5f){
        this->state*=-1;
        this->animation_start=now;
    }
    if(now-this->plant_time>=this->ttl) this->should_explode=true;
}

Tile::Tile(int coord_x, int coord_y, float size){
    this->coord_x=coord_x;
    this->coord_y=coord_y;
    this->size=size;
}

void Tile::setPosition(float x, float y) {
    this->pos_x=x;
    this->pos_y=y;
}

void Tile::addPlayer(Boomerman player) {
    this->players.push_back(player);
}

void Tile::removePlayer(Boomerman player) {
    int to_be_removed = player.id;
    auto it = std::find_if(this->players.begin(), this->players.end(), [to_be_removed](const Boomerman& obj) {
        return obj.id == to_be_removed;});
    if(it != this->players.end()) this->players.erase(it);
}