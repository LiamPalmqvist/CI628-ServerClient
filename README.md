# Multiplayer Pong project for CI628
This is a simple multiplayer Pong game developed as part of the CI628 course. The game allows two players to compete against each other in real-time over a network connection.

## Features
- Real-time multiplayer gameplay
- Simple and intuitive controls
- Score tracking

## Technologies Used
- C++ for game logic
- SDL2 for graphics and input handling
- sys/socket for networking

## Getting Started
To get started with the project, follow these steps:
1. Clone the repository with `git clone`
2. Navigate to the project directory: `cd CI628-ClientServer`
3. Create a build directory: `mkdir build && cd build`
4. Generate the Makefile using CMake: `cmake ..`
5. Run the server: `./Client server`
6. Run the client: `./Client`
7. Enjoy playing Pong!

## Controls
- Player: W (up), S (down)

## License
This project is licensed under the GPL 2.0 License. See the LICENSE file for details.

## Acknowledgments
- Thanks to the CI628 course instructors for their guidance and support.
- SDL2 library for providing excellent tools for game development.
- Inspiration from classic Pong games.
- Feel free to contribute to the project by submitting issues or pull requests!
- Happy coding!