#include "Player.h"

Player::Player(int startX, int startY, int startHP, int startSpeed)
    : x(startX), y(startY), hp(startHP), speed(startSpeed) {}

void Player::moveUp()    { --y; }
void Player::moveDown()  { ++y; }
void Player::moveLeft()  { --x; }
void Player::moveRight() { ++x; }

int  Player::getX() const { return x; }
int  Player::getY() const { return y; }
int  Player::getHP() const { return hp; }
int  Player::getSpeed() const { return speed; }

void Player::setPos(int nx, int ny) { x = nx; y = ny; }

void Player::takeDamage(int dmg) {
    hp -= dmg;
    if (hp < 0) hp = 0;
}

bool Player::isAlive() const { return hp > 0; }
