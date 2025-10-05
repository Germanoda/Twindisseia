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
      npc(0, 0),
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
        init_pair(1, COLOR_RED,   -1); // enemy red
        init_pair(2, COLOR_GREEN, -1); // NPC green
    }

    auto seed = static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);

    lastMessage = "Explore the map. Move with WASD/Arrows, press Q to quit. Step on 'g' to battle or 'N' to talk.";
    spawnEnemy();
    spawnNPC();
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

void Game::spawnNPC() {
    for (int tries = 0; tries < 1000; ++tries) {
        int nx = 1 + (rng() % (map.getWidth()  - 2));
        int ny = 1 + (rng() % (map.getHeight() - 2));
        if (!map.isWalkable(nx, ny)) continue;
        if ((nx == player.getX() && ny == player.getY()) ||
            (nx == enemy.getX()  && ny == enemy.getY()))
            continue;
        npc.setPos(nx, ny);
        return;
    }
    npc.setPos(map.getWidth()-2, map.getHeight()-2);
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
    bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
    lastMessage = playerTurn
        ? "Combat started! You act first."
        : "Combat started! Enemy acts first.";

    // Intro frame
    clear();
    map.draw();
    if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
        if (has_colors()) attron(COLOR_PAIR(1));
        mvaddch(enemy.getY(), enemy.getX(), 'g');
        if (has_colors()) attroff(COLOR_PAIR(1));
    }
    if (onScreen(npc.getX(), npc.getY())) {
        if (has_colors()) attron(COLOR_PAIR(2));
        mvaddch(npc.getY(), npc.getX(), 'N');
        if (has_colors()) attroff(COLOR_PAIR(2));
    }
    if (onScreen(player.getX(), player.getY()))
        mvaddch(player.getY(), player.getX(), '@');
    drawUI();
    refresh();
    napms(450);

    // Turn loop
    while (player.isAlive() && enemy.isAlive()) {
        if (playerTurn) {
            int dmg = rollD6();
            enemy.takeDamage(dmg);
            lastMessage = "You attack! You roll " + std::to_string(dmg) + ".";
        } else {
            int dmg = rollD6();
            player.takeDamage(dmg);
            lastMessage = "Enemy attacks! Enemy rolls " + std::to_string(dmg) + ".";
        }

        clear();
        map.draw();
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2));
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2));
        }
        if (onScreen(player.getX(), player.getY()))
            mvaddch(player.getY(), player.getX(), '@');
        drawUI();
        refresh();
        napms(450);

        if (!player.isAlive() || !enemy.isAlive()) break;
        playerTurn = !playerTurn;
    }

    if (!player.isAlive()) {
        lastMessage = "You died! Press any key to exit.";
        clear();
        map.draw();
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2));
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2));
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

void Game::talkToNPC() {
    // Modal dialogue: advance one line per key press.
    nodelay(stdscr, FALSE);

    const auto& lines = npc.getDialog();
    for (size_t i = 0; i < lines.size(); ++i) {
        clear();
        map.draw();

        // Draw entities
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2));
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2));
        }
        if (onScreen(player.getX(), player.getY()))
            mvaddch(player.getY(), player.getX(), '@');

        // Dialogue box (reuse HUD area)
        int uiY = std::min(map.getHeight(), std::max(0, LINES - 3));
        if (uiY >= 0 && uiY < LINES) {
            mvprintw(uiY, 0, "[NPC] %s", lines[i].c_str());
            clrtoeol();
        }
        if (uiY + 1 < LINES) {
            mvprintw(uiY + 1, 0, "(Press any key to continue...)");
            clrtoeol();
        }
        refresh();

        getch(); // wait for key to advance
    }

    // Exit hint
    clear();
    map.draw();
    if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
        if (has_colors()) attron(COLOR_PAIR(1));
        mvaddch(enemy.getY(), enemy.getX(), 'g');
        if (has_colors()) attroff(COLOR_PAIR(1));
    }
    if (onScreen(npc.getX(), npc.getY())) {
        if (has_colors()) attron(COLOR_PAIR(2));
        mvaddch(npc.getY(), npc.getX(), 'N');
        if (has_colors()) attroff(COLOR_PAIR(2));
    }
    if (onScreen(player.getX(), player.getY()))
        mvaddch(player.getY(), player.getX(), '@');

    int uiY = std::min(map.getHeight(), std::max(0, LINES - 2));
    if (uiY >= 0 && uiY < LINES) {
        mvprintw(uiY, 0, "The NPC nods. Farewell.");
        clrtoeol();
    }
    if (uiY + 1 < LINES) {
        mvprintw(uiY + 1, 0, "(Press any key to continue)");
        clrtoeol();
    }
    refresh();
    getch();

    nodelay(stdscr, TRUE);
    lastMessage = "You talked to the NPC.";
}

bool Game::tryMovePlayer(int dx, int dy) {
    int nx = player.getX() + dx;
    int ny = player.getY() + dy;

    if (!map.isWalkable(nx, ny)) return false;

    // If target tile has NPC -> talk (then move into the tile)
    if (nx == npc.getX() && ny == npc.getY()) {
        talkToNPC();
        player.setPos(nx, ny);
        return true;
    }

    // If target tile has enemy -> combat
    if (enemy.isAlive() && enemy.getX() == nx && enemy.getY() == ny) {
        startCombat();
        if (player.isAlive() && !enemy.isAlive()) {
            player.setPos(nx, ny);
        }
        return true;
    }

    // Normal movement
    player.setPos(nx, ny);
    return true;
}

void Game::run() {
    while (running) {
        clear();

        map.draw();

        // Enemy (red)
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1));
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1));
        }

        // NPC (green)
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2));
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2));
        }

        // Player
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
