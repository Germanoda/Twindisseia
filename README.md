Twindisseia is a terminal-based ASCII adventure game written in C++ using ncurses.

## Build & Run (Linux)
1. Install dependencies
   sudo apt update
   sudo apt install -y build-essential libncurses5-dev libncursesw5-dev

2. Build
   make

3. Run
   make run

4. Clean build files
   make clean

## Gameplay
- Move your character around the map.
- Encounter enemies in random positions.
- Turn-based combat based on Speed (the fastest attacks first).
- Defeat enemies to survive — when your HP reaches zero, a defeat message appears.