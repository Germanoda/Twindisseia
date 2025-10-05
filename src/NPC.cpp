#include "NPC.h"

NPC::NPC(int startX, int startY, std::vector<std::string> dialog)
    : x(startX), y(startY), lines(std::move(dialog)) {}

int  NPC::getX() const { return x; }
int  NPC::getY() const { return y; }
void NPC::setPos(int nx, int ny) { x = nx; y = ny; }

const std::vector<std::string>& NPC::getDialog() const {
    return lines;
}
