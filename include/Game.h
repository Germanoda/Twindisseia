#ifndef GAME_H
#define GAME_H

#include <string>
#include <random>
#include <vector>
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

    // Mensagens/UI
    std::string lastMessage;

    // RNG para dados
    std::mt19937 rng;
    std::uniform_int_distribution<int> d6;

    // --- UI helpers (novos) ---
    void drawHUD() const;                      // barra superior com HP e dicas
    void drawMessageBox(const std::string&) const; // caixa com borda e wrap
    static std::vector<std::string> wrapText(const std::string& s, int maxw);

    // Combate e jogo
    int  rollD6();
    void spawnEnemy();
    void spawnNPC();
    void startCombat();
    void talkToNPC();
    bool tryMovePlayer(int dx, int dy);

public:
    Game();
    void run();
};

#endif
