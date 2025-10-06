#pragma once
#include <string>
#include <vector>
#include <ncurses.h>
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"

struct UiWindows {
  WINDOW *hud=nullptr, *mapw=nullptr, *side=nullptr, *msg=nullptr;
};

class Ui {
public:
  Ui(int sidebarWidth=18, int msgHeight=9);
  ~Ui();

  void layout();  // call on start and on KEY_RESIZE
  void renderFrame(const Map& map, const Player& player,
                   const Enemy& enemy, const NPC& npc,
                   const std::string& message, bool showIndicator=false);

  bool onMapViewport(int x, int y) const;
  WINDOW* mapWindow() const;

private:
  int sidebarWidth, msgHeight;
  UiWindows w;

  void destroy();

  void drawHUD(const Player& player, const Enemy& enemy) const;
  void drawSidebar(const Player& player, const Enemy& enemy) const;
  void drawMessageBox(const std::string& text, bool showIndicator) const;

  // ⬇️ declare exactly as defined in Ui.cpp
  static std::vector<std::string> wrapText(const std::string& s, int maxw);
};
