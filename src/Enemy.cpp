#include "Enemy.h"

Enemy::Enemy(int startX, int startY, int startHP, int startSpeed)
    : x(startX), y(startY), hp(startHP), speed(startSpeed), alive(true) {}

int Enemy::getX() const { return x; }
int Enemy::getY() const { return y; }
int Enemy::getHP() const { return hp; }
int Enemy::getSpeed() const { return speed; }
bool Enemy::isAlive() const { return alive; }

void Enemy::setPos(int nx, int ny) { x = nx; y = ny; }

void Enemy::takeDamage(int dmg) {
    if (!alive) return;
    hp -= dmg;
    if (hp <= 0) {
        hp = 0;
        alive = false;
    }
}
