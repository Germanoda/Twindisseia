#ifndef GAME_H
#define GAME_H

#include <string>
#include <random>
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include "NPC.h"

class Game {
private:
    bool running;
    Map map;
    Player player;
    Enemy enemy;
    NPC npc;

    std::string lastMessage;

    std::mt19937 rng;
    std::uniform_int_distribution<int> d6;

    int  rollD6();                 // 1..6
    void drawUI() const;           // HUD
    void spawnEnemy();             // place enemy
    void spawnNPC();               // place friendly NPC
    void startCombat();            // turn-based combat by speed
    void talkToNPC();              // modal dialog with the NPC
    bool tryMovePlayer(int dx, int dy);

public:
    Game();
    void run();
};

#endif
