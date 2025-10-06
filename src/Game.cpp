#include "Game.h"
#include <chrono>
#include <ncurses.h>

Game::Game()
: map(30, 15),
  player(map.getWidth()/2, map.getHeight()/2),
  enemy(0, 0),
  npc(0, 0),
  d6(1, 6),        // <-- move d6 before combat
  ui(18, 5),
  combat(rng)
{
  // ncurses base
  initscr();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  timeout(50); // non-blocking input with ~20 FPS

  // colors
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED,   -1); // enemy
    init_pair(2, COLOR_GREEN, -1); // npc
    init_pair(3, COLOR_CYAN,  -1); // player
    init_pair(5, COLOR_WHITE, -1); // text/hud
  }

  // rng seed
  auto seed = static_cast<unsigned>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  rng.seed(seed);

  // first message
  lastMessage = "Explore the map. Move with WASD/Arrows, press Q to quit. Step on 'g' to battle, 'N' to talk.";

  // place actors
  spawnEnemy();
  spawnNPC();
  // --- Starting gear for PLAYER ---
  // Sword: adds +1d8 to damage
  Equipment sword;
  sword.name = "Sword";
  sword.slot = EquipSlot::Weapon;
  sword.attackDice = { {1,8} };

  // Helmet: reduce incoming by 1d2
  Equipment helmet;
  helmet.name = "Helmet";
  helmet.slot = EquipSlot::Helmet;
  helmet.defenseDice = { {1,2} };

  // Chest plate: reduce incoming by (1 + 1d4)
  Equipment chest;
  chest.name = "Chest Plate";
  chest.slot = EquipSlot::Chest;
  chest.flatDefBonus = 1;           // the "+1" part
  chest.defenseDice  = { {1,4} };   // the "d4" part

  // Boots: +2 speed, reduce incoming by 1d2
  Equipment boots;
  boots.name = "Boots";
  boots.slot = EquipSlot::Boots;
  boots.spdBonus = 2;
  boots.defenseDice = { {1,2} };

  player.setWeapon(sword);
  player.setHelmet(helmet);
  player.setChest(chest);
  player.setBoots(boots);

  // --- Starting gear for ENEMY ---
  // Mace: adds +2d4 to damage
  Equipment mace;
  mace.name = "Mace";
  mace.slot = EquipSlot::Weapon;
  mace.attackDice = { {2,4} };

  // Helmet & Chest equal to the player's (no boots)
  enemy.setWeapon(mace);
  enemy.setHelmet(helmet);
  enemy.setChest(chest);

  // create windows
  ui.layout();
}

Game::~Game() {
  endwin(); // Ui destructor already deletes windows; this restores terminal
}

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
        (nx == enemy.getX()  && ny == enemy.getY())) continue;
    npc.setPos(nx, ny);
    return;
  }
  npc.setPos(map.getWidth()-2, map.getHeight()-2);
}

bool Game::tryMovePlayer(int dx, int dy) {
  int nx = player.getX() + dx;
  int ny = player.getY() + dy;

  if (!map.isWalkable(nx, ny)) return false;

  // NPC: talk, then step into tile
  if (nx == npc.getX() && ny == npc.getY()) {
    dialog.run(npc, map, player, enemy, ui, lastMessage);
    player.setPos(nx, ny);
    return true;
  }

  // Enemy: battle, then step into tile if you win
  if (enemy.isAlive() && enemy.getX() == nx && enemy.getY() == ny) {
    combat.run(map, player, enemy, npc, ui, running, lastMessage);
    if (running && player.isAlive() && !enemy.isAlive()) {
      player.setPos(nx, ny);
    }
    return true;
  }

  // normal move
  player.setPos(nx, ny);
  return true;
}

void Game::run() {
  while (running) {
    int ch = getch();
    if (ch == KEY_RESIZE) {
      ui.layout(); // recreate/resize windows
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

    ui.renderFrame(map, player, enemy, npc, lastMessage, /*showIndicator=*/false);
  }
}
