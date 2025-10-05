#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include <ncurses.h>

class Map {
private:
    int width, height;
    std::vector<std::string> grid;

public:
    Map(int w = 20, int h = 10);

    // Draw the map rows into a target window.
    // Renders up to the window's size (no overflow).
    void drawTo(WINDOW* win) const;

    bool isWalkable(int x, int y) const;
    int getWidth()  const;
    int getHeight() const;
};

#endif
