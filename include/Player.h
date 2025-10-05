#ifndef PLAYER_H
#define PLAYER_H

class Player {
private:
    int x, y;
    int hp;
    int speed; // determines turn order

public:
    Player(int startX = 0, int startY = 0, int startHP = 10, int startSpeed = 5);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    int  getX() const;
    int  getY() const;
    int  getHP() const;
    int  getSpeed() const;

    void setPos(int nx, int ny);

    void takeDamage(int dmg);
    bool isAlive() const;
};

#endif
