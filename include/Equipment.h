#pragma once
#include <string>
#include <vector>

struct Dice {
    int count = 0;  // e.g., 2 for "2d4"
    int sides = 0;  // e.g., 4 for "2d4"
};

enum class EquipSlot { Weapon, Helmet, Chest, Boots };

struct Equipment {
    std::string name;
    EquipSlot   slot;

    // dice modifiers
    std::vector<Dice> attackDice;   // extra damage dice (added to d6)
    std::vector<Dice> defenseDice;  // damage reduction dice (subtracted)

    // flat bonuses
    int flatDefBonus = 0;           // flat reduction to incoming damage
    int spdBonus     = 0;           // speed bonus (Boots)
};
