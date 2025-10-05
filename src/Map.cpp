#include "Map.h"
#include <algorithm>

Map::Map(int w, int h) : width(w), height(h), grid(h, std::string(w, '.')) {
    // walls (border)
    for (int i = 0; i < width; ++i) {
        grid[0][i] = '#';
        grid[height - 1][i] = '#';
    }
    for (int i = 0; i < height; ++i) {
        grid[i][0] = '#';
        grid[i][width - 1] = '#';
    }
}

void Map::drawTo(WINDOW* win) const {
    if (!win) return;
    int h = 0, w = 0;
    getmaxyx(win, h, w);

    const int rows = std::min(height, h);
    const int cols = std::min(width,  w);
    for (int y = 0; y < rows; ++y) {
        // draw only the visible slice
        mvwaddnstr(win, y, 0, grid[y].c_str(), cols);
    }
}

bool Map::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return false;
    return grid[y][x] != '#';
}

int Map::getWidth()  const { return width;  }
int Map::getHeight() const { return height; }
