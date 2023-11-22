#ifndef BOOMBERMAN_WALL_H
#define BOOMBERMAN_WALL_H

#include <memory>

class Wall{
private:
    int pos_x, pos_y;
    bool is_destructible;
public:
    Wall(int pos_x, int pos_y, bool is_descructible);
    [[nodiscard]] std::unique_ptr<int[]> getPosition() const;
};


#endif //BOOMBERMAN_WALL_H
