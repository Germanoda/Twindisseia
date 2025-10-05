#include "DialogueSystem.h"

void DialogueSystem::run(const NPC& npc, Map& map,
                         const Player& player, const Enemy& enemy,
                         Ui& ui, std::string& lastMessage) {
  nodelay(stdscr, FALSE);

  const auto& lines = npc.getDialog();
  for (size_t i = 0; i < lines.size(); ++i) {
    lastMessage = "[NPC] " + lines[i];
    ui.renderFrame(map, player, enemy, npc, lastMessage, i == 0); // indicator only on first
    getch();
  }

  lastMessage = "You talked to the NPC.";
  ui.renderFrame(map, player, enemy, npc, lastMessage, false);
  getch();

  nodelay(stdscr, TRUE);
}
