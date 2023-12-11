#include <algorithm>
#include "EntityHandler.h"
#include "raylib.h"

void EntityHandler::placeBomb(int x, int y, int explosion, int size, long double timestamp, int ttl, bool is_atomic) {
    this->bombs.emplace_back(x, y, explosion, size, timestamp, ttl, is_atomic);
}

void EntityHandler::drawPlayers(Map *map) {
    for (auto &player: this->players) {
        player.animateHit();
        if(player.getState() == 1) DrawRectangle(player.getBoombermanPos()[0] * map->offset + map->start_x+3, player.getBoombermanPos()[1] * map->offset + map->start_y+3, map->size-6,map->size-6, player.color);
    }
}

void EntityHandler::drawBombs(Map* map) {
    Color c;
    for (auto &bomb: this->bombs) {
        if(!bomb.ShouldExplode()) {
            bomb.animate();
            if (bomb.state == -1) c = BLACK;
            else{
              if(bomb.is_atomic) c = GREEN;
              else c = RED;
            }
            DrawRectangle(bomb.pos_x * map->offset + map->start_x + 7, bomb.pos_y * map->offset + map->start_y + 7,
                          map->size - 14, map->size - 14, c);
        }
        else this->explodeBomb(&bomb, map);
    }
}

int EntityHandler::explodeBomb(Bomb* bomb, Map* map) {
    std::vector<TileOnFire> tilesExploded;
    int x=bomb->pos_x, y=bomb->pos_y;
    double long fuze_time=bomb->plant_time;
    auto it = std::find_if(this->bombs.begin(), this->bombs.end(), [x,y,fuze_time](Bomb& bomb) {
        return bomb.pos_x == x && bomb.pos_y == y && bomb.plant_time == fuze_time;
    });
    if (it != this->bombs.end()) {
        tilesExploded = bomb->boom(map);
        this->theFloorIsLava.insert(this->theFloorIsLava.begin(),tilesExploded.begin(), tilesExploded.end());
        this->bombs.erase(it);
        printf("Bomb at x:%d y:%d exploded!\n",bomb->pos_x,bomb->pos_y);
        return 0;
    }
    return 1;
}

void EntityHandler::drawFire(Map *map) {
    for (auto tile: this->theFloorIsLava) {
        DrawRectangle(tile.x * map->offset + map->start_x, tile.y * map->offset + map->start_y, map->size,map->size, RED);
    }
}

void EntityHandler::tryExtinguish() {
    for(size_t i=0; i<this->theFloorIsLava.size(); i++){
        if(this->theFloorIsLava[i].shouldCalmDown()) this->theFloorIsLava.erase(this->theFloorIsLava.begin()+i);
    }
}