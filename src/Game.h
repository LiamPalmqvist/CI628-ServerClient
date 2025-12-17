//
// Created by Liam on 29/11/2025.
//

#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H


#include <iostream>
#include <SDL2_mixer/SDL_mixer.h>

// Since base gameObjects don't need to be initialised,
// and it will only ever be inherited,
// We don't need an initialiser for this class
class GameObject
{
public:
    // Variables
    int _xPos;
    int _yPos;
    int _speed;

    // Functions
    [[nodiscard]] int get_x_pos() const { return _xPos; }
    [[nodiscard]] int get_y_pos() const { return _yPos; }
    [[nodiscard]] int get_speed() const { return _speed; }

    void set_x_pos(const int xPos) { _xPos = xPos; }
    void set_y_pos(const int yPos) { _yPos = yPos; }
    void set_speed(const int speed) { _speed = speed; }
};

class Paddle : public GameObject
{
    // Private variables
    int _xSize;
    int _ySize;

public:
    // Public initialiser
    explicit Paddle(int player);
    Paddle(int xPos, int yPos, int xSize, int ySize);

    // Public functions
    [[nodiscard]] int get_xSize() const { return _xSize; }
    [[nodiscard]] int get_ySize() const { return _ySize; }

    void set_xSize(const int xSize) { _xSize = xSize; }
    void set_ySize(const int ySize) { _ySize = ySize; }
};

class Ball : public GameObject
{
    // Private variables
    int _radius;
    int _direction;
public:
    // Public initialiser
    Ball();
    Ball(int xPos, int yPos, int size);

    // Public functions
    [[nodiscard]] int get_radius() const { return _radius; }
    [[nodiscard]] int get_direction() const { return _direction; }

    void set_radius(const int radius) { _radius = radius; }
    void set_direction(const int direction) { _direction = direction; }
    void set_xy_pos(const int xPos, const int yPos) { _xPos = xPos; _yPos = yPos; }
};

class Game
{
    // Private methods
    void checkCollisions();

public:
    // Public variables

    // Game objects
    Paddle playerPaddles[2] = {Paddle(0), Paddle(1)};
    Ball ball;
    bool playerKeys[4] = {false, false, false, false};
    int p1Points = 0;
    int p2Points = 0;
    bool playHitSound = false;

    bool playing;

    Mix_Chunk *ballHitSound = NULL;

    // Public initialiser
    Game();

    // Public functions and methods
    void encodeData(int* outBuffer) const;
    void decodeData(int* data);
    void printData() const;
    void update();
};


#endif //CLIENT_GAME_H