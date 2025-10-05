#ifndef NPC_H
#define NPC_H

#include <vector>
#include <string>

class NPC {
private:
    int x, y;
    std::vector<std::string> lines;

public:
    NPC(int startX = 0, int startY = 0,
        std::vector<std::string> dialog = {
            "Hello, traveler.",
            "These halls are dangerous.",
            "Trust your instincts and your dice."
        });

    int  getX() const;
    int  getY() const;
    void setPos(int nx, int ny);

    const std::vector<std::string>& getDialog() const;
};

#endif
