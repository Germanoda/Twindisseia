#ifndef ENEMY_H
#define ENEMY_H

class Enemy {
private:
    int x, y;
    int hp;
    int speed;
    bool alive;

public:
    Enemy(int startX = 0, int startY = 0, int startHP = 6, int startSpeed = 3);

    int  getX() const;
    int  getY() const;
    int  getHP() const;
    int  getSpeed() const;
    bool isAlive() const;

    void setPos(int nx, int ny);
    void takeDamage(int dmg);
};

#endif
