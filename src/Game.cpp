#include "Game.h"
#include <chrono>
#include <algorithm>

Game::Game()
    : running(true),
      map(30, 15),
      player(map.getWidth()/2, map.getHeight()/2, 10, 5), // HP 10, SPD 5
      enemy(0, 0, 6, 3),
      npc(0, 0),
      d6(1, 6)
{
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(50); // non-blocking input + frame delay

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_RED,    -1); // enemy
        init_pair(2, COLOR_GREEN,  -1); // NPC
        init_pair(3, COLOR_CYAN,   -1); // player
        init_pair(5, COLOR_WHITE,  -1); // text/HUD
    }

    auto seed = static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);

    lastMessage = "Explore the map. Move with WASD/Arrows, press Q to quit. Step on 'g' to battle, 'N' to talk.";
    spawnEnemy();
    spawnNPC();
    layoutWindows();
}

Game::~Game() {
    destroyWindows();
    endwin();
}

void Game::destroyWindows() {
    if (hud)  { delwin(hud);  hud  = nullptr; }
    if (mapw) { delwin(mapw); mapw = nullptr; }
    if (side) { delwin(side); side = nullptr; }
    if (msg)  { delwin(msg);  msg  = nullptr; }
}

void Game::layoutWindows() {
    destroyWindows();

    // Clamp layout to available space
    int H = LINES;
    int W = COLS;

    int hudH  = 1;
    int sideW = std::min(sidebarWidth, std::max(0, W / 2)); // don't exceed half screen if tiny
    int mapW  = std::max(0, W - sideW);
    int msgH  = std::min(msgHeight, std::max(0, H / 3));    // limit height if tiny
    int mapH  = std::max(0, H - hudH - msgH);

    // Create windows
    hud  = newwin(hudH, W,    0,    0);
    mapw = newwin(mapH, mapW, hudH, 0);
    side = newwin(H - hudH, sideW, hudH, mapW);
    msg  = newwin(msgH, mapW, hudH + mapH, 0);

    // Enable keypad per window (optional)
    keypad(mapw, TRUE);
    keypad(side, TRUE);
    keypad(msg,  TRUE);
}

bool Game::onMapViewport(int x, int y) const {
    if (!mapw) return false;
    int h=0, w=0;
    getmaxyx(mapw, h, w);
    return (x >= 0 && y >= 0 && x < w && y < h);
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

/* ---------- UI ---------- */

void Game::drawHUD() const {
    if (!hud) return;
    werase(hud);
    // bar
    wattron(hud, A_REVERSE | (has_colors() ? COLOR_PAIR(5) : 0));
    wmove(hud, 0, 0);
    wprintw(hud, "HP:%d  Enemy:%d  SPD:%d  |  Move: WASD/Arrows  Q:Quit",
            player.getHP(), enemy.isAlive() ? enemy.getHP() : 0, player.getSpeed());
    wattroff(hud, A_REVERSE | (has_colors() ? COLOR_PAIR(5) : 0));
}

std::vector<std::string> Game::wrapText(const std::string& s, int maxw) {
    std::vector<std::string> out;
    if (maxw <= 0) return out;
    int i = 0, n = (int)s.size();
    while (i < n) {
        int len = std::min(maxw, n - i);
        int breakPos = -1;
        for (int j = 0; j < len; ++j)
            if (s[i + j] == ' ') breakPos = j;
        if (len == maxw && breakPos != -1) len = breakPos + 1;
        std::string piece = s.substr(i, len);
        while (!piece.empty() && piece.back() == ' ') piece.pop_back();
        out.push_back(piece);
        i += (int)piece.size();
        if (i < n && s[i] == ' ') ++i;
    }
    return out;
}

void Game::drawMessageBox(const std::string& text, bool showIndicator) const {
    if (!msg) return;
    werase(msg);
    box(msg, 0, 0);

    int h=0, w=0;
    getmaxyx(msg, h, w);
    const int innerW = std::max(0, w - 2);

    auto lines = wrapText(text, innerW);
    for (int i = 0; i < (int)lines.size() && i < h - 2; ++i) {
        mvwaddnstr(msg, 1 + i, 1, lines[i].c_str(), innerW);
    }

    if (showIndicator && h >= 2 && w >= 2) {
        wattron(msg, A_BOLD | (has_colors() ? COLOR_PAIR(5) : 0));
        mvwaddch(msg, h - 2, w - 2, '>');
        wattroff(msg, A_BOLD | (has_colors() ? COLOR_PAIR(5) : 0));
    }
}

void Game::drawSidebar() const {
    if (!side) return;
    werase(side);

    int h=0, w=0;
    getmaxyx(side, h, w);

    // vertical divider is the window border at column 0 (we'll leave it blank and indent content)
    int cx = 1, cy = 0; // content area

    auto print = [&](const std::string& s){
        if (cy < h) mvwaddnstr(side, cy++, cx, s.c_str(), std::max(0, w - cx - 1));
    };

    print("== STATUS ==");
    print("Player");
    print("  HP: " + std::to_string(player.getHP()));
    print("  SPD: " + std::to_string(player.getSpeed()));
    cy++;

    print("Enemy");
    print(std::string("  HP: ") + (enemy.isAlive() ? std::to_string(enemy.getHP()) : "0"));
    print("  SPD: " + std::to_string(enemy.getSpeed()));
    cy++;

    print("Keys");
    print("  Move: WASD/Arrows");
    print("  Quit: Q");
}

/* ---------- Frame render ---------- */
void Game::render(bool showIndicator) const {
    // HUD
    drawHUD();

    // MAP
    if (mapw) {
        werase(mapw);
        map.drawTo(mapw);

        // entities
        auto drawEntity = [&](int x, int y, chtype ch, short pair){
            if (onMapViewport(x, y)) {
                if (pair && has_colors()) wattron(mapw, COLOR_PAIR(pair) | A_BOLD);
                mvwaddch(mapw, y, x, ch);
                if (pair && has_colors()) wattroff(mapw, COLOR_PAIR(pair) | A_BOLD);
            }
        };
        if (enemy.isAlive()) drawEntity(enemy.getX(), enemy.getY(), 'g', 1);
        drawEntity(npc.getX(),   npc.getY(),   'N', 2);
        drawEntity(player.getX(),player.getY(),'@', 3);
    }

    // SIDEBAR
    drawSidebar();

    // MESSAGE
    drawMessageBox(lastMessage, showIndicator);

    // batch refresh
    if (hud)  wnoutrefresh(hud);
    if (mapw) wnoutrefresh(mapw);
    if (side) wnoutrefresh(side);
    if (msg)  wnoutrefresh(msg);
    doupdate();
}

/* ---------- Gameplay ---------- */

void Game::startCombat() {
    bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
    lastMessage = playerTurn ? "Combat started! You act first."
                             : "Combat started! Enemy acts first.";
    render(false);

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
        render(false);

        if (!player.isAlive() || !enemy.isAlive()) break;
        playerTurn = !playerTurn;
    }

    if (!player.isAlive()) {
        lastMessage = "You died! Press any key to exit.";
        render(true);
        // block once to show defeat screen
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
        running = false;
        return;
    }

    lastMessage = "You defeated the enemy! (+Victory)";
    render(false);
}

void Game::talkToNPC() {
    nodelay(stdscr, FALSE);

    const auto& lines = npc.getDialog();
    for (size_t i = 0; i < lines.size(); ++i) {
        lastMessage = "[NPC] " + lines[i];
        // show “press key” indicator only on the first line
        render(i == 0);
        getch();
    }

    lastMessage = "You talked to the NPC.";
    render(false);
    getch();

    nodelay(stdscr, TRUE);
}

bool Game::tryMovePlayer(int dx, int dy) {
    int nx = player.getX() + dx;
    int ny = player.getY() + dy;

    if (!map.isWalkable(nx, ny)) return false;

    // talk if stepping onto NPC
    if (nx == npc.getX() && ny == npc.getY()) {
        talkToNPC();
        player.setPos(nx, ny);
        return true;
    }

    // battle if stepping onto enemy
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
        // Handle input (non-blocking; timeout set in constructor)
        int ch = getch();
        if (ch == KEY_RESIZE) {
            layoutWindows();
        } else {
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
        }

        // Draw everything (no napms needed thanks to timeout)
        render(false);
    }
}
