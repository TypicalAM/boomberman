#ifndef BOOMBERMAN_BOOMBERMAN_H
#define BOOMBERMAN_BOOMBERMAN_H

#include "../shared/game/Map.h"
#include "../shared/Util.h"
#include <memory>
#include <string>

class Boomberman{
private:
    int* position;
    int health, state;
    double long animation_start{};
public:
    int iframes;
    std::string pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy;
    Boomberman(const std::string& pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy, int start_x, int start_y, int health);
    std::unique_ptr<int[]> getBoombermanPos();
    void setBoombermanPos(int new_x, int new_y);
    void move(Map *map,std::shared_ptr<int[]> curr_pos, int x, int y);
    void cleanUp();
    void gotHit();
    void decrementIframes();
    void animateHit();
    int getState() const;
};

#endif //BOOMBERMAN_BOOMBERMAN_H
