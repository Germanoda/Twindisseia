#pragma once
#include "Equipment.h"

class Enemy {
private:
    int x, y;
    int hp;
    int speed;
    int attack;
    int defense;
    bool alive;

    // gear
    Equipment weapon; // Weapon
    Equipment helmet; // Helmet
    Equipment chest;  // Chest
    // (no boots for enemy per your spec)

public:
    Enemy(int startX = 0, int startY = 0,
          int startHP = 6, int startSpeed = 3,
          int startAttack = 1, int startDefense = 0);

    int  getX() const;
    int  getY() const;
    int  getHP() const;
    int  getSpeed() const;   // base (no boots), could add later
    int  getAttack() const;
    int  getDefense() const;
    bool isAlive() const;

    void setPos(int nx, int ny);
    void takeDamage(int dmg);

    // gear access / setters
    const Equipment& getWeapon() const { return weapon; }
    const Equipment& getHelmet() const { return helmet; }
    const Equipment& getChest()  const { return chest;  }

    void setWeapon(const Equipment& e) { weapon = e; }
    void setHelmet(const Equipment& e) { helmet = e; }
    void setChest (const Equipment& e) { chest  = e; }
};
