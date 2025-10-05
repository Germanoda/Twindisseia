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
        init_pair(1, COLOR_RED,   -1); // enemy
        init_pair(2, COLOR_GREEN, -1); // NPC
        init_pair(3, COLOR_CYAN,  -1); // player (opcional)
        init_pair(4, COLOR_YELLOW,-1); // highlight/dice
        init_pair(5, COLOR_WHITE, -1); // HUD/text
    }

    auto seed = static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);

    lastMessage = "Explore the map. Move with WASD/Arrows, press Q to quit. Step on 'g' to battle and 'N' to talk.";
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

/* ---------- UI: HUD + Message Box com borda e wrap ---------- */

void Game::drawHUD() const {
    // HUD vai na linha 0 (acima do mapa), se houver espaço.
    if (LINES <= 0) return;
    move(0, 0);
    clrtoeol();

    // Tenta montar uma faixa com infos essenciais
    std::string hud = "HP: " + std::to_string(player.getHP()) +
                      "  |  Enemy: " + std::to_string(enemy.isAlive() ? enemy.getHP() : 0) +
                      "  |  SPD " + std::to_string(player.getSpeed()) +
                      "  |  Move: WASD/Arrows  |  Q: Quit";

    // Estilo leve
    attr_t style = A_REVERSE;
    if (has_colors()) style |= COLOR_PAIR(5);
    attron(style);
    mvaddnstr(0, 0, hud.c_str(), std::max(0, COLS));
    attroff(style);
}

std::vector<std::string> Game::wrapText(const std::string& s, int maxw) {
    std::vector<std::string> lines;
    if (maxw <= 0) return lines;

    int i = 0, n = (int)s.size();
    while (i < n) {
        int len = std::min(maxw, n - i);
        int breakPos = -1;

        // tenta quebrar em espaço
        for (int j = 0; j < len; ++j) {
            if (s[i + j] == ' ') breakPos = j;
        }

        if (len == maxw && breakPos != -1) len = breakPos + 1; // quebra “bonita”

        std::string piece = s.substr(i, len);
        // tira espaços à direita
        while (!piece.empty() && piece.back() == ' ') piece.pop_back();
        lines.push_back(piece);

        i += (int)piece.size();
        // pula um espaço se existir
        if (i < n && s[i] == ' ') ++i;
    }
    return lines;
}

void Game::drawMessageBox(const std::string& msg) const {
    // Caixa com borda logo ABAIXO do mapa, ocupando até 3 linhas de texto.
    int top = map.getHeight() + 1;        // linha inicial da caixa
    if (top + 4 >= LINES) {               // precisa de 5 linhas (borda + 3 linhas + borda)
        // fallback: escreve sem borda, uma linha só
        if (top < LINES) {
            mvprintw(top, 0, "%s", msg.c_str());
            clrtoeol();
        }
        return;
    }

    int w = std::min(map.getWidth(), COLS);
    int h = 5; // topo, 3 linhas de conteúdo, base

    // Desenha bordas usando ACS_*
    // canto superior esquerdo
    mvaddch(top, 0, ACS_ULCORNER);
    // linha horizontal superior
    mvhline(top, 1, ACS_HLINE, w - 2);
    // canto superior direito
    mvaddch(top, w - 1, ACS_URCORNER);

    // laterais
    for (int y = 1; y < h - 1; ++y) {
        if (top + y >= LINES) break;
        mvaddch(top + y, 0, ACS_VLINE);
        mvaddch(top + y, w - 1, ACS_VLINE);
        // limpa miolo
        if (w > 2) {
            move(top + y, 1);
            for (int x = 1; x < w - 1; ++x) addch(' ');
        }
    }

    // base
    if (top + h - 1 < LINES) {
        mvaddch(top + h - 1, 0, ACS_LLCORNER);
        mvhline(top + h - 1, 1, ACS_HLINE, w - 2);
        mvaddch(top + h - 1, w - 1, ACS_LRCORNER);
    }

    // Conteúdo (wrap em até 3 linhas)
    int innerW = std::max(0, w - 2);
    auto wrapped = wrapText(msg, innerW);
    for (int i = 0; i < (int)wrapped.size() && i < 3; ++i) {
        if (has_colors()) attron(COLOR_PAIR(5));
        mvaddnstr(top + 1 + i, 1, wrapped[i].c_str(), innerW);
        if (has_colors()) attroff(COLOR_PAIR(5));
    }
}

/* ------------------- Combate e Diálogo ------------------- */

void Game::startCombat() {
    bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
    lastMessage = playerTurn
        ? "Combat started! You act first."
        : "Combat started! Enemy acts first.";

    // Quadro de introdução
    clear();
    drawHUD();
    map.draw();

    if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
        if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
        mvaddch(enemy.getY(), enemy.getX(), 'g');
        if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
    }
    if (onScreen(npc.getX(), npc.getY())) {
        if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
        mvaddch(npc.getY(), npc.getX(), 'N');
        if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);
    }
    if (onScreen(player.getX(), player.getY())) {
        if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
        mvaddch(player.getY(), player.getX(), '@');
        if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
    }

    drawMessageBox(lastMessage);
    refresh();
    napms(450);

    // Loop de turnos
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
        drawHUD();
        map.draw();

        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
        }
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);
        }
        if (onScreen(player.getX(), player.getY())) {
            if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(player.getY(), player.getX(), '@');
            if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
        }

        drawMessageBox(lastMessage);
        refresh();
        napms(450);

        if (!player.isAlive() || !enemy.isAlive()) break;
        playerTurn = !playerTurn;
    }

    if (!player.isAlive()) {
        lastMessage = "You died! Press any key to exit.";
        clear();
        drawHUD();
        map.draw();

        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
        }
        if (onScreen(player.getX(), player.getY())) {
            if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(player.getY(), player.getX(), '@');
            if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
        }

        drawMessageBox(lastMessage);
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
    nodelay(stdscr, FALSE);

    const auto& lines = npc.getDialog();
    for (size_t i = 0; i < lines.size(); ++i) {
        clear();
        drawHUD();
        map.draw();

        // Enemy (red)
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
        }
        // NPC (green)
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);
        }
        // Player (cyan)
        if (onScreen(player.getX(), player.getY())) {
            if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(player.getY(), player.getX(), '@');
            if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
        }

        // Só o primeiro diálogo mostra a dica de tecla
        std::string box = "[NPC] " + lines[i];
        if (i == 0) box += "  (Press any key to continue)";

        drawMessageBox(box);
        refresh();
        getch(); // ainda avança uma fala por tecla
    }

    // Quadro de saída do diálogo sem "press any key"
    clear();
    drawHUD();
    map.draw();

    if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
        if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
        mvaddch(enemy.getY(), enemy.getX(), 'g');
        if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
    }
    if (onScreen(npc.getX(), npc.getY())) {
        if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
        mvaddch(npc.getY(), npc.getX(), 'N');
        if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);
    }
    if (onScreen(player.getX(), player.getY())) {
        if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
        mvaddch(player.getY(), player.getX(), '@');
        if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
    }

    lastMessage = "You talked to the NPC.";
    drawMessageBox(lastMessage);  // sem "(Press any key...)"
    refresh();
    getch(); // mantém a pausa final, sem exibir a dica

    nodelay(stdscr, TRUE);
}

bool Game::tryMovePlayer(int dx, int dy) {
    int nx = player.getX() + dx;
    int ny = player.getY() + dy;

    if (!map.isWalkable(nx, ny)) return false;

    // Talk
    if (nx == npc.getX() && ny == npc.getY()) {
        talkToNPC();
        player.setPos(nx, ny);
        return true;
    }

    // Battle
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

        // HUD sempre no topo
        drawHUD();

        // Mapa logo abaixo do HUD (se o terminal for baixo, o mapa já faz crop)
        map.draw();

        // Enemy (red)
        if (enemy.isAlive() && onScreen(enemy.getX(), enemy.getY())) {
            if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
            mvaddch(enemy.getY(), enemy.getX(), 'g');
            if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);
        }

        // NPC (green)
        if (onScreen(npc.getX(), npc.getY())) {
            if (has_colors()) attron(COLOR_PAIR(2) | A_BOLD);
            mvaddch(npc.getY(), npc.getX(), 'N');
            if (has_colors()) attroff(COLOR_PAIR(2) | A_BOLD);
        }

        // Player (cyan)
        if (onScreen(player.getX(), player.getY())) {
            if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(player.getY(), player.getX(), '@');
            if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);
        }

        // Caixa de mensagem abaixo do mapa
        drawMessageBox(lastMessage);

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
