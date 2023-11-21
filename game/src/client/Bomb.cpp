#include "Bomb.h"
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