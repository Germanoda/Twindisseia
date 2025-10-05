#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>

class Map {
private:
    int width, height;
    std::vector<std::string> grid;

public:
    Map(int w = 20, int h = 10);
    void draw() const;
    bool isWalkable(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
};

#endif
