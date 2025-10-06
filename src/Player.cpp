#include "Player.h"
#include <algorithm>

Player::Player(int startX, int startY, int startHP, int startSpeed,
               int startAttack, int startDefense)
    : x(startX), y(startY),
      baseHP(startHP), baseSpeed(startSpeed),
      baseAttack(startAttack), baseDefense(startDefense),
      hp(startHP)
{
    // gear starts empty; Game will assign starting equipment
}

void Player::moveUp()    { --y; }
void Player::moveDown()  { ++y; }
void Player::moveLeft()  { --x; }
void Player::moveRight() { ++x; }
void Player::setPos(int nx, int ny) { x = nx; y = ny; }

int  Player::getX() const { return x; }
int  Player::getY() const { return y; }

int  Player::getHP() const { return hp; }
int  Player::getSpeed() const { return baseSpeed + boots.spdBonus; }
int  Player::getAttack() const { return baseAttack; }
int  Player::getDefense() const { return baseDefense; }

bool Player::isAlive() const { return hp > 0; }

void Player::takeDamage(int dmg) {
    hp -= dmg;
    if (hp < 0) hp = 0;
}
