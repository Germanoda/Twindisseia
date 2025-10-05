#include "Ui.h"
#include <algorithm>

Ui::Ui(int sidebarWidth, int msgHeight)
: sidebarWidth(sidebarWidth), msgHeight(msgHeight) {
  w.hud = w.mapw = w.side = w.msg = nullptr;
}

Ui::~Ui() { destroy(); }

void Ui::destroy() {
  if (w.hud)  { delwin(w.hud);  w.hud  = nullptr; }
  if (w.mapw) { delwin(w.mapw); w.mapw = nullptr; }
  if (w.side) { delwin(w.side); w.side = nullptr; }
  if (w.msg)  { delwin(w.msg);  w.msg  = nullptr; }
}

void Ui::layout() {
  destroy();
  int H = LINES, W = COLS;

  int hudH  = 1;
  int sideW = std::min(sidebarWidth, std::max(0, W / 2));
  int mapW  = std::max(0, W - sideW);
  int msgH  = std::min(msgHeight, std::max(0, H / 3));
  int mapH  = std::max(0, H - hudH - msgH);

  w.hud  = newwin(hudH, W,       0,          0);
  w.mapw = newwin(mapH, mapW,    hudH,       0);
  w.side = newwin(H - hudH, sideW, hudH,     mapW);
  w.msg  = newwin(msgH, mapW,    hudH+mapH,  0);

  keypad(w.mapw, TRUE);
  keypad(w.side, TRUE);
  keypad(w.msg,  TRUE);
}

WINDOW* Ui::mapWindow() const { return w.mapw; }

bool Ui::onMapViewport(int x, int y) const {
  if (!w.mapw) return false;
  int h=0, ww=0;
  getmaxyx(w.mapw, h, ww);
  return (x >= 0 && y >= 0 && x < ww && y < h);
}

std::vector<std::string> Ui::wrapText(const std::string& s, int maxw) {
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

void Ui::drawHUD(const Player& player, const Enemy& enemy) const {
  if (!w.hud) return;
  werase(w.hud);
  wattron(w.hud, A_REVERSE | (has_colors() ? COLOR_PAIR(5) : 0));
  mvwprintw(w.hud, 0, 0, "HP:%d  Enemy:%d  SPD:%d  |  Move: WASD/Arrows  Q:Quit",
            player.getHP(), enemy.isAlive() ? enemy.getHP() : 0, player.getSpeed());
  wattroff(w.hud, A_REVERSE | (has_colors() ? COLOR_PAIR(5) : 0));
}

void Ui::drawSidebar(const Player& player, const Enemy& enemy) const {
  if (!w.side) return;
  werase(w.side);
  int h=0, ww=0; getmaxyx(w.side, h, ww);
  int cx = 1, cy = 0;
  auto print = [&](const std::string& s){
    if (cy < h) mvwaddnstr(w.side, cy++, cx, s.c_str(), std::max(0, ww - cx - 1));
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

void Ui::drawMessageBox(const std::string& text, bool showIndicator) const {
  if (!w.msg) return;
  werase(w.msg);
  box(w.msg, 0, 0);
  int h=0, ww=0; getmaxyx(w.msg, h, ww);
  const int innerW = std::max(0, ww - 2);
  auto lines = wrapText(text, innerW);
  for (int i = 0; i < (int)lines.size() && i < h - 2; ++i) {
    mvwaddnstr(w.msg, 1 + i, 1, lines[i].c_str(), innerW);
  }
  if (showIndicator && h >= 2 && ww >= 2) {
    wattron(w.msg, A_BOLD | (has_colors() ? COLOR_PAIR(5) : 0));
    mvwaddch(w.msg, h - 2, ww - 2, '>');
    wattroff(w.msg, A_BOLD | (has_colors() ? COLOR_PAIR(5) : 0));
  }
}

void Ui::renderFrame(const Map& map, const Player& player,
                     const Enemy& enemy, const NPC& npc,
                     const std::string& message, bool showIndicator) {
  // HUD
  drawHUD(player, enemy);

  // MAP
  if (w.mapw) {
    werase(w.mapw);
    map.drawTo(w.mapw);

    auto drawEntity = [&](int x, int y, chtype ch, short pair){
      if (onMapViewport(x, y)) {
        if (pair && has_colors()) wattron(w.mapw, COLOR_PAIR(pair) | A_BOLD);
        mvwaddch(w.mapw, y, x, ch);
        if (pair && has_colors()) wattroff(w.mapw, COLOR_PAIR(pair) | A_BOLD);
      }
    };
    if (enemy.isAlive()) drawEntity(enemy.getX(), enemy.getY(), 'g', 1);
    drawEntity(npc.getX(),   npc.getY(),   'N', 2);
    drawEntity(player.getX(),player.getY(),'@', 3);
  }

  // SIDEBAR + MSG
  drawSidebar(player, enemy);
  drawMessageBox(message, showIndicator);

  if (w.hud)  wnoutrefresh(w.hud);
  if (w.mapw) wnoutrefresh(w.mapw);
  if (w.side) wnoutrefresh(w.side);
  if (w.msg)  wnoutrefresh(w.msg);
  doupdate();
}
