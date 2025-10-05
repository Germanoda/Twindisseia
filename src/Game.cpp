#include <ncurses.h>
#include <chrono>
#include <algorithm>
#include "Game.h"

static inline bool onScreen(int x, int y) {
    return (y >= 0 && y < LINES && x >= 0 && x < COLS);
}

Game::Game()
    : running(true),
      map(30, 15),
      player(map.getWidth() / 2, map.getHeight() / 2, 10, 5), // HP 10, Speed 5
      enemy(0, 0, 6, 3),                                      // HP 6,  Speed 3
      d6(1, 6)
{
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_RED, -1); // enemy color: red
    }

    auto seed = static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);

    lastMessage = "Explore the map. Move with WASD/Arrows, press Q to quit. Step on 'g' to start a battle.";
    spawnEnemy();
}

int Game::rollD6() { return d6(rng); }

void Game::spawnEnemy() {
    for (int tries = 0; tries < 1000; ++tries) {
        int ex = 1 + (rng() % (map.getWidth()  - 2));
        int ey = 1 + (rng() % (map.getHeight() - 2));
        if (!map.isWalkable(ex, ey)) continue;
        if (ex == player.getX() && ey == player.getY()) continue;
        enemy.setPos(ex, ey);
        return;
    }
    enemy.setPos(1, 1);
}

void Game::drawUI() const {
    int uiY = std::min(map.getHeight(), std::max(0, LINES - 2));
    if (uiY >= 0 && uiY < LINES) {
        mvprintw(uiY, 0, "Player HP: %d (SPD %d)   Enemy HP: %d (SPD %d)   ",
                 player.getHP(), player.getSpeed(),
                 enemy.isAlive() ? enemy.getHP() : 0, enemy.getSpeed());
        clrtoeol();
    }
    if (uiY + 1 < LINES) {
        mvprintw(uiY + 1, 0, "%s", lastMessage.c_str());
        clrtoeol();
    }
}

void Game::startCombat() {
    // Determine initial turn order by speed (tie -> player first)
    bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
    lastMessage = playerTurn
        ? "Combat started! You act first."
        : "Combat started! Enemy acts first.";

    // small intro frame
    clear();
    map.draw();
    if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
        if (has_colors()) attron(COLOR_PAIR(1));
        mvaddch(enemy.getY(), enemy.getX(), 'g');
        if (has_colors()) attroff(COLOR_PAIR(1));
    }
    if (onScreen(player.getX(), player.getY()))
        mvaddch(player.getY(), player.getX(), '@');
    drawUI();
    refresh();
    napms(450);

    // Turn-based loop
    while (player.isAlive() && enemy.isAlive()) {
        // --- Attacker's turn ---
        if (playerTurn) {
            int dmg = rollD6();
            enemy.takeDamage(dmg);
            lastMessage = "You attack! You roll " + std::to_string(dmg) + ".";
        } else {
            int dmg = rollD6();
            player.takeDamage(dmg);
            lastMessage = "Enemy attacks! Enemy rolls " + std::to_string(dmg) + ".";
        }

        // Draw after each attack
        clear();
        map.draw();
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(player.getX(), player.getY()))
            mvaddch(player.getY(), player.getX(), '@');
        drawUI();
        refresh();
        napms(450);

        // Check death after the attack
        if (!player.isAlive()) break;
        if (!enemy.isAlive()) break;

        // Swap turn
        playerTurn = !playerTurn;
    }

    // End of combat
    if (!player.isAlive()) {
        // Defeat screen
        lastMessage = "You died! Press any key to exit.";
        clear();
        map.draw();
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(player.getX(), player.getY()))
            mvaddch(player.getY(), player.getX(), '@');
        drawUI();
        refresh();

        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);

        running = false;
        return;
    }

    lastMessage = "You defeated the enemy! (+Victory)";
}

bool Game::tryMovePlayer(int dx, int dy) {
    int nx = player.getX() + dx;
    int ny = player.getY() + dy;

    if (!map.isWalkable(nx, ny)) return false;

    if (enemy.isAlive() && enemy.getX() == nx && enemy.getY() == ny) {
        startCombat();
        if (player.isAlive() && !enemy.isAlive()) {
            player.setPos(nx, ny);
        }
        return true;
    }

    player.setPos(nx, ny);
    return true;
}

void Game::run() {
    while (running) {
        clear();

        map.draw();
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(player.getX(), player.getY()))
            mvaddch(player.getY(), player.getX(), '@');

        drawUI();
        refresh();

        int ch = getch();
        switch (ch) {
            case 'q': running = false; break;
            case KEY_UP:
            case 'w': tryMovePlayer(0, -1); break;
            case KEY_DOWN:
            case 's': tryMovePlayer(0,  1); break;
            case KEY_LEFT:
            case 'a': tryMovePlayer(-1, 0); break;
            case KEY_RIGHT:
            case 'd': tryMovePlayer(1,  0); break;
            default: break;
        }

        napms(50);
    }

    endwin();
}
