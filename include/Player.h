#pragma once
#include "Equipment.h"

class Player {
private:
    int x, y;
    // base stats
    int baseHP;
    int baseSpeed;
    int baseAttack;
    int baseDefense;

    // current HP (separate from baseHP for future max HP handling)
    int hp;

    // equipped gear
    Equipment weapon; // Weapon
    Equipment helmet; // Helmet
    Equipment chest;  // Chest
    Equipment boots;  // Boots

public:
    Player(int startX = 0, int startY = 0,
           int startHP = 10, int startSpeed = 5,
           int startAttack = 2, int startDefense = 1);

    // position / movement
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void setPos(int nx, int ny);
    int  getX() const;
    int  getY() const;

    // effective combat stats
    int  getHP() const;
    int  getSpeed() const;    // base + boots.spdBonus
    int  getAttack() const;   // baseAttack (gear dice handled in CombatSystem)
    int  getDefense() const;  // baseDefense (flat reductions handled in CombatSystem)
    bool isAlive() const;
    void takeDamage(int dmg);

    // gear access
    const Equipment& getWeapon() const { return weapon; }
    const Equipment& getHelmet() const { return helmet; }
    const Equipment& getChest()  const { return chest;  }
    const Equipment& getBoots()  const { return boots;  }

    // equip setters (used in Game to give starting gear)
    void setWeapon(const Equipment& e) { weapon = e; }
    void setHelmet(const Equipment& e) { helmet = e; }
    void setChest (const Equipment& e) { chest  = e; }
    void setBoots (const Equipment& e) { boots  = e; }
};
