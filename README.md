⚔️ Windisseia
A lightweight, terminal-based ASCII adventure game written in C++ using the ncurses library. Explore, fight, and survive in a world rendered entirely in your terminal!

A screenshot or GIF of the game in action would look great here!

✨ Features
Classic ASCII Aesthetics: A retro-style adventure that runs in any modern terminal.

Dynamic Encounters: Enemies appear in random positions, making each playthrough unique.

Turn-Based Combat: A strategic combat system where the character with the highest Speed statistic attacks first.

Simple & Lightweight: Written in C++ with minimal dependencies for a fast and clean experience.

🚀 Getting Started
Follow these instructions to get a copy of the project up and running on your local machine.

Prerequisites
This project is designed for Linux-based systems. You will need g++, make, and the ncurses development libraries.

Installation & Building
Clone the repository (if you are using Git):

Bash

git clone https://github.com/your_username/windisseia.git
cd windisseia
Install dependencies (for Debian/Ubuntu-based systems):

Bash

sudo apt update && sudo apt install -y build-essential libncurses5-dev libncursesw5-dev
Build the project:
Compile the source code using the provided Makefile.

Bash

make
🎮 How to Play
Running the Game
After a successful build, you can run the game with a simple command:

Bash

make run
This will launch the game in your current terminal window.

Gameplay
Objective: Your goal is to survive by defeating the enemies you encounter. If your HP (Health Points) drops to zero, the game is over.

Movement: Use the arrow keys or WASD to move your character (@) around the map.

Combat: When you move into an enemy's tile, combat begins. Attacks are turn-based and ordered by the Speed stat. Outmaneuver and defeat your foes to continue your journey.

Cleaning Up
To remove the compiled binary and object files, run:

Bash

make clean
🗺️ Future Roadmap
This is just the beginning! Here are some ideas for future development:

[ ] An inventory and item system

[ ] More enemy types with unique abilities

[ ] Dungeons, towns, and friendly NPCs

[ ] A persistent story or quest line

[ ] Character stats and a leveling system

🤝 Contributing
Contributions are welcome! If you have ideas for new features or bug fixes, feel free to fork the repository and submit a pull request.

📜 License
This project is licensed under the MIT License - see the LICENSE.md file for details.