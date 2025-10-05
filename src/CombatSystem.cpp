#include "CombatSystem.h"

CombatSystem::CombatSystem(std::mt19937& rng) : rng(rng) {}

int CombatSystem::rollD6() {
  static std::uniform_int_distribution<int> d6(1, 6);
  return d6(rng);
}

void CombatSystem::run(Map& map, Player& player, Enemy& enemy, NPC& npc,
                       Ui& ui, bool& running, std::string& lastMessage) {
  bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
  lastMessage = playerTurn ? "Combat started! You act first."
                           : "Combat started! Enemy acts first.";
  ui.renderFrame(map, player, enemy, npc, lastMessage, false);

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
    ui.renderFrame(map, player, enemy, npc, lastMessage, false);

    if (!player.isAlive() || !enemy.isAlive()) break;
    playerTurn = !playerTurn;
  }

  if (!player.isAlive()) {
    lastMessage = "You died! Press any key to exit.";
    ui.renderFrame(map, player, enemy, npc, lastMessage, true);
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
    running = false;
    return;
  }

  lastMessage = "You defeated the enemy! (+Victory)";
  ui.renderFrame(map, player, enemy, npc, lastMessage, false);
}
