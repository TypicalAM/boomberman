#include <cstdio>
#include "Bomb.h"

Bomb::Bomb(int pos_x, int pos_y, int explosion, int size, float ttl, bool is_atomic) {
    this->pos_x=pos_x;
    this->pos_y=pos_y;
    this->explosion=explosion;
    this->size=size;
    this->ttl=ttl;
    this->animation_start=Util::TimestampMillis();
    this->plant_time=this->animation_start;
    this->state=-1;
    this->is_atomic=is_atomic;
}

bool Bomb::ShouldExplode() const {
    return Util::TimestampMillis() > this->plant_time + this->ttl*1000;
}

void Bomb::animateOrBoom() {
    double long now=Util::TimestampMillis();
    if(now-this->animation_start>=500.0f){
        this->state*=-1;
        this->animation_start=now;
    }
}
std::vector<TileOnFire> Bomb::boom(Map* map) {
    std::vector<TileOnFire> tilesLitUp;
    tilesLitUp.emplace_back(this->pos_x,this->pos_y);
    for(int direction=-1; direction<=1; direction+=2) {
        for (int x = 1; x < this->explosion; x++) {
            int may_be_wall = map->getSquareState(this->pos_x + x*direction, this->pos_y);
            if (may_be_wall==1) break;
            else if(may_be_wall==2){
                map->setSquareState(this->pos_x + x*direction,this->pos_y,0);
                tilesLitUp.emplace_back(this->pos_x+x*direction, this->pos_y);
                if(!this->is_atomic) break;
            }
            else tilesLitUp.emplace_back(this->pos_x+x*direction, this->pos_y);
        }
        for (int y = 1; y < this->explosion; y++) {
            int may_be_wall = map->getSquareState(this->pos_x, this->pos_y+y*direction);
            if (may_be_wall==1) break;
            else if(may_be_wall==2){
                map->setSquareState(this->pos_x,this->pos_y+y*direction,0);
                tilesLitUp.emplace_back(this->pos_x, this->pos_y+y*direction);
                if(!this->is_atomic) break;
            }
            else tilesLitUp.emplace_back(this->pos_x, this->pos_y+y*direction);
        }
    }
    return tilesLitUp;
}
