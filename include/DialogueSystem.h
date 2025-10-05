#pragma once
#include <string>
#include "NPC.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Ui.h"

// Simple modal dialogue: one line per key press.
// Only the first line shows a "press key" indicator.
class DialogueSystem {
public:
  void run(const NPC& npc, Map& map,
           const Player& player, const Enemy& enemy,
           Ui& ui, std::string& lastMessage);
};
