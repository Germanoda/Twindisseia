#pragma once
#include <string>
#include <random>
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "NPC.h"
#include "Ui.h"
#include "CombatSystem.h"
#include "DialogueSystem.h"

class Game {
public:
  Game();
  ~Game();
  void run();

private:
  // world & actors
  Map map;
  Player player;
  Enemy enemy;
  NPC npc;

  // game state
  bool running = true;
  std::string lastMessage;

  // rng FIRST (so it's constructed before CombatSystem references it)
  std::mt19937 rng;
  std::uniform_int_distribution<int> d6;

  // systems AFTER rng
  Ui ui;                 // windows + rendering
  CombatSystem combat;   // turn-based dice combat
  DialogueSystem dialog; // npc dialogue

  // setup
  void spawnEnemy();
  void spawnNPC();

  // input helpers
  bool tryMovePlayer(int dx, int dy);
};
