#ifndef GAME_H
#define GAME_H

#include <string>
#include <random>
#include <vector>
#include <ncurses.h>
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "Map.h"

class Game {
private:
    bool running;
    Map map;
    Player player;
    Enemy enemy;
    NPC npc;

    // UI state
    std::string lastMessage;

    // RNG
    std::mt19937 rng;
    std::uniform_int_distribution<int> d6;

    // Layout
    int sidebarWidth = 18; // right panel width
    int msgHeight     = 5; // message box height

    // Windows (HUD, MAP, SIDEBAR, MESSAGE)
    WINDOW *hud = nullptr, *mapw = nullptr, *side = nullptr, *msg = nullptr;

    // --- window management ---
    void layoutWindows();   // create/resize windows based on current COLS/LINES
    void destroyWindows();  // cleanup

    // --- UI helpers ---
    void drawHUD() const;
    void drawSidebar() const;
    void drawMessageBox(const std::string& text, bool showIndicator = false) const;
    static std::vector<std::string> wrapText(const std::string& s, int maxw);

    // Map-viewport bound check for entity draw on mapw
    bool onMapViewport(int x, int y) const;

    // --- game helpers ---
    int  rollD6();
    void spawnEnemy();
    void spawnNPC();
    void startCombat();
    void talkToNPC();
    bool tryMovePlayer(int dx, int dy);

    // --- render frame (map+entities+hud+side+msg) ---
    void render(bool showIndicator = false) const;

public:
    Game();
    ~Game();
    void run();
};

#endif
