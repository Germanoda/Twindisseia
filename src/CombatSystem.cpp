#include "CombatSystem.h"
#include <ncurses.h>
#include <sstream>
#include <vector>
#include <random>

// --- small helpers ---
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

static int rollDiceListSum(std::mt19937& rng, const std::vector<Dice>& list) {
  int sum = 0;
  for (const auto& d : list) sum += rollDice(rng, d.count, d.sides);
  return sum;
}

// dmg = max(1, (baseD6 + ATK + atkDiceSum) - (DEF + flatDef + defDiceSum))
static int computeDamage(int baseD6, int atk, int atkDiceSum,
                         int targetDef, int flatDef, int defDiceSum) {
  int offense = baseD6 + atk + atkDiceSum;
  int defense = targetDef + flatDef + defDiceSum;
  int dmg = offense - defense;
  if (dmg < 1) dmg = 1;
  return dmg;
}

void CombatSystem::run(Map& map, Player& player, Enemy& enemy, NPC& npc,
                       Ui& ui, bool& running, std::string& lastMessage) {
  bool playerTurn = (player.getSpeed() >= enemy.getSpeed());
  lastMessage = playerTurn ? "Combat started! You act first."
                           : "Combat started! Enemy acts first.";
  ui.renderFrame(map, player, enemy, npc, lastMessage, /*indicator*/true);
  wait_key_and_restore_timeout();

  while (player.isAlive() && enemy.isAlive()) {
    if (playerTurn) {
      lastMessage = "You attack! Rolling...";
      ui.renderFrame(map, player, enemy, npc, lastMessage, true);
      napms(250);

      int base = rollD6();
      const auto& w  = player.getWeapon();
      const auto& eh = enemy.getHelmet();
      const auto& ec = enemy.getChest();

      int atkDiceSum = rollDiceListSum(rng, w.attackDice);
      int defDiceSum = rollDiceListSum(rng, eh.defenseDice) +
                       rollDiceListSum(rng, ec.defenseDice);
      int flatRed    = eh.flatDefBonus + ec.flatDefBonus;

      int dmg = computeDamage(base, player.getAttack(), atkDiceSum,
                              enemy.getDefense(), flatRed, defDiceSum);
      enemy.takeDamage(dmg);

      std::ostringstream os;
      os << "You attack: d6=" << base
         << " + atk=" << player.getAttack()
         << " + w=" << atkDiceSum
         << "  vs  def=" << enemy.getDefense()
         << " + flat=" << flatRed
         << " + arm=" << defDiceSum
         << " -> " << dmg << " dmg.";
      lastMessage = os.str();

    } else {
      lastMessage = "Enemy attacks! Rolling...";
      ui.renderFrame(map, player, enemy, npc, lastMessage, true);
      napms(250);

      int base = rollD6();
      const auto& w  = enemy.getWeapon();
      const auto& ph = player.getHelmet();
      const auto& pc = player.getChest();
      const auto& pb = player.getBoots();

      int atkDiceSum = rollDiceListSum(rng, w.attackDice);
      int defDiceSum = rollDiceListSum(rng, ph.defenseDice) +
                       rollDiceListSum(rng, pc.defenseDice) +
                       rollDiceListSum(rng, pb.defenseDice);
      int flatRed    = ph.flatDefBonus + pc.flatDefBonus + pb.flatDefBonus;

      int dmg = computeDamage(base, enemy.getAttack(), atkDiceSum,
                              player.getDefense(), flatRed, defDiceSum);
      player.takeDamage(dmg);

      std::ostringstream os;
      os << "Enemy attack: d6=" << base
         << " + atk=" << enemy.getAttack()
         << " + w=" << atkDiceSum
         << "  vs  def=" << player.getDefense()
         << " + flat=" << flatRed
         << " + arm=" << defDiceSum
         << " -> " << dmg << " dmg.";
      lastMessage = os.str();
    }

    ui.renderFrame(map, player, enemy, npc, lastMessage, /*indicator*/true);
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
