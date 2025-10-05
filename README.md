# Twindisseia

Twindisseia is a terminal-based ASCII adventure game written in C++ using ncurses.
Battle enemies, explore maps, and survive in a retro-style dungeon — all rendered directly in your terminal.

![build](https://github.com/Germanoda/Twindisseia/actions/workflows/build.yml/badge.svg)

## Build & Run (Linux)
1. Install dependencies
   ```bash
   sudo apt update
   sudo apt install -y build-essential libncurses5-dev libncursesw5-dev
   ```

2. Build
   ```bash
   make
   ```

3. Run
   ```bash
   make run
   ```

4. Clean build files
   ```bash
   make clean
   ```

## Gameplay
- Move your character around the map.
- Encounter enemies in random positions.
- Turn-based combat based on Speed (the fastest attacks first).
- Defeat enemies to survive — when your HP reaches zero, a defeat message appears.

## License
This project is released under the MIT License.
See the LICENSE file for more information.

## Author
**Germanoda**  
GitHub: [https://github.com/Germanoda](https://github.com/Germanoda)
