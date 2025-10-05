#include <ncurses.h>
#include "Map.h"

Map::Map(int w, int h) : width(w), height(h), grid(h, std::string(w, '.')) {
    // bordas
    for (int i = 0; i < width; ++i) {
        grid[0][i] = '#';
        grid[height - 1][i] = '#';
    }
    for (int i = 0; i < height; ++i) {
        grid[i][0] = '#';
        grid[i][width - 1] = '#';
    }
}

void Map::draw() const {
    for (int y = 0; y < height; ++y) {
        mvprintw(y, 0, "%s", grid[y].c_str());
    }
}

bool Map::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return false;
    return grid[y][x] != '#';
}

int Map::getWidth() const  { return width; }
int Map::getHeight() const { return height; }
