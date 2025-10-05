#pragma once
#include <random>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Map.h"
#include "NPC.h"
#include "Ui.h"

// Turn-based, speed-ordered, dice combat.
class CombatSystem {
public:
  explicit CombatSystem(std::mt19937& rng);
  // Runs the fight; updates lastMessage each step.
  // Sets `running=false` if the player dies (so Game can exit).
  void run(Map& map, Player& player, Enemy& enemy, NPC& npc,
           Ui& ui, bool& running, std::string& lastMessage);

private:
  std::mt19937& rng;
  int rollD6();
};
