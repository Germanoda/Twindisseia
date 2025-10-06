#include "CombatSystem.h"
#include <ncurses.h>

static inline void wait_key_and_restore_timeout() {
  nodelay(stdscr, FALSE);
  flushinp();
  getch();
  timeout(50);
}

CombatSystem::CombatSystem(std::mt19937& rng) : rng(rng) {}

int CombatSystem::rollD6() {
  static std::uniform_int_distribution<int> d6(1, 6);
  return d6(rng);
}

static int rollDice(std::mt19937& rng, int count, int sides) {
  if (count <= 0 || sides <= 0) return 0;
  std::uniform_int_distribution<int> dist(1, sides);
  int sum = 0;
  for (int i = 0; i < count; ++i) sum += dist(rng);
  return sum;
}

static int rollDiceList(std::mt19937& rng, const std::vector<Dice>& list) {
  int sum = 0;
  for (const auto& d : list) sum += rollDice(rng, d.count, d.sides);
  return sum;
}

// Compute final damage using: d6 + ATK + attackDice - (DEF + flatDef + defenseDice)
// then clamp to at least 1 so fights can't stall.
static int computeDamage(std::mt19937& rng,
                         int baseRoll, int atk,
                         const std::vector<Dice>& attackDice,
                         int targetDef, int targetFlatDef,
                         const std::vector<Dice>& targetDefDice) {
  int rollPart   = baseRoll + rollDiceList(rng, attackDice);
  int defenseRed = targetDef + targetFlatDef + rollDiceList(rng, targetDefDice);
  int dmg = rollPart + atk - defenseRed;
  if (dmg < 1) dmg = 1;
  return dmg;
}

void CombatSystem::run(Map& map, Player& player, Enemy& enemy, NPC& npc,
                       Ui& ui, bool& running, std::string& lastMessage) {
  bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
  lastMessage = playerTurn ? "Combat started! You act first."
                           : "Combat started! Enemy acts first.";

  ui.renderFrame(map, player, enemy, npc, lastMessage, true);
  wait_key_and_restore_timeout();

  while (player.isAlive() && enemy.isAlive()) {
    if (playerTurn) {
      lastMessage = "You attack! Rolling...";
      ui.renderFrame(map, player, enemy, npc, lastMessage, true);
      napms(300);

      // Player attack
      int base = rollD6();
      const auto& w  = player.getWeapon();
      const auto& eh = enemy.getHelmet();
      const auto& ec = enemy.getChest();

      int flatRed = 0;
      flatRed += eh.flatDefBonus;
      flatRed += ec.flatDefBonus;

      std::vector<Dice> defDice;
      if (!eh.name.empty()) defDice.insert(defDice.end(), eh.defenseDice.begin(), eh.defenseDice.end());
      if (!ec.name.empty()) defDice.insert(defDice.end(), ec.defenseDice.begin(), ec.defenseDice.end());

      int dmg = computeDamage(rng,
                              base, player.getAttack(),
                              w.attackDice,
                              enemy.getDefense(), flatRed,
                              defDice);

      enemy.takeDamage(dmg);
      lastMessage = "You roll d6=" + std::to_string(base) +
                    " + ATK " + std::to_string(player.getAttack()) +
                    (w.attackDice.empty() ? "" : " + weapon dice") +
                    "  vs enemy DEF " + std::to_string(enemy.getDefense()) +
                    " => " + std::to_string(dmg) + " damage.";
    } else {
      lastMessage = "Enemy attacks! Rolling...";
      ui.renderFrame(map, player, enemy, npc, lastMessage, true);
      napms(300);

      // Enemy attack
      int base = rollD6();
      const auto& w  = enemy.getWeapon();
      const auto& ph = player.getHelmet();
      const auto& pc = player.getChest();
      const auto& pb = player.getBoots();

      int flatRed = 0;
      flatRed += ph.flatDefBonus;
      flatRed += pc.flatDefBonus;
      flatRed += pb.flatDefBonus; // boots may have flat reduction (we'll set 0; dice only)

      std::vector<Dice> defDice;
      if (!ph.name.empty()) defDice.insert(defDice.end(), ph.defenseDice.begin(), ph.defenseDice.end());
      if (!pc.name.empty()) defDice.insert(defDice.end(), pc.defenseDice.begin(), pc.defenseDice.end());
      if (!pb.name.empty()) defDice.insert(defDice.end(), pb.defenseDice.begin(), pb.defenseDice.end());

      int dmg = computeDamage(rng,
                              base, enemy.getAttack(),
                              w.attackDice,
                              player.getDefense(), flatRed,
                              defDice);

      player.takeDamage(dmg);
      lastMessage = "Enemy rolls d6=" + std::to_string(base) +
                    " + ATK " + std::to_string(enemy.getAttack()) +
                    (w.attackDice.empty() ? "" : " + weapon dice") +
                    "  vs your DEF " + std::to_string(player.getDefense()) +
                    " => " + std::to_string(dmg) + " damage.";
    }

    ui.renderFrame(map, player, enemy, npc, lastMessage, true);
    wait_key_and_restore_timeout();

    if (!player.isAlive() || !enemy.isAlive()) break;
    playerTurn = !playerTurn;
  }

  if (!player.isAlive()) {
    lastMessage = "You died! Press any key to exit.";
    ui.renderFrame(map, player, enemy, npc, lastMessage, true);
    wait_key_and_restore_timeout();
    running = false;
    return;
  }

  lastMessage = "You defeated the enemy! (+Victory)";
  ui.renderFrame(map, player, enemy, npc, lastMessage, false);
}
