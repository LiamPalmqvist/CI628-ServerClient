//
// Created by Liam on 25/11/2025.
//

#include "Game.h"
#include <ostream>


Paddle::Paddle(const int player) : GameObject()
{
    if (player == 0)
        _xPos = 30;
    else
        _xPos = 770;
    _yPos = 300;
    _xSize = 10;
    _ySize = 100;
    _speed = 10;
}

Paddle::Paddle(const int xPos, const int yPos, const int xSize, const int ySize) : GameObject()
{
    _xPos = xPos;
    _yPos = yPos;
    _xSize = xSize;
    _ySize = ySize;
}

Ball::Ball() : GameObject()
{
    _xPos = 300;
    _yPos = 200;
    _radius = 10;
    _direction = 180.0f;
    _speed = 5;
}

Ball::Ball(const int xPos, const int yPos, const int size) : GameObject()
{
    _xPos = xPos;
    _yPos = yPos;
    _radius = size;
    _direction = 20;
    _speed = 10;
}

Game::Game()
{
    playing = false;
}

void Game::update()
{
    playHitSound = false;

    // First, we check how many points each player has
    // if either player has 10 points, end the game
    if (p1Points >= 9)
    {
        playing = false;
        return;
    }
    if (p2Points >= 9)
    {
        playing = false;
        return;
    }


    // std::cout << playerKeys[0] << " . " << playerKeys[1] << std::endl;
    if (playerKeys[0])
    {
        // std::cout << "was " << playerPaddles[0].get_y_pos() << std::endl;
        playerPaddles[0].set_y_pos(playerPaddles[0].get_y_pos()-playerPaddles[0].get_speed());
        // std::cout << "is now " << playerPaddles[0].get_y_pos() << std::endl;
    }
    if (playerKeys[1])
    {
        // std::cout << "was " << playerPaddles[0].get_y_pos() << std::endl;
        playerPaddles[0].set_y_pos(playerPaddles[0].get_y_pos()+playerPaddles[0].get_speed());
        // std::cout << "is now " << playerPaddles[0].get_y_pos() << std::endl;
    }
    if (playerKeys[2])
    {
        // std::cout << "was " << playerPaddles[1].get_y_pos() << std::endl;
        playerPaddles[1].set_y_pos(playerPaddles[1].get_y_pos()-playerPaddles[1].get_speed());
        // std::cout << "is now " << playerPaddles[1].get_y_pos() << std::endl;
    }
    if (playerKeys[3])
    {
        // std::cout << "was " << playerPaddles[1].get_y_pos() << std::endl;
        playerPaddles[1].set_y_pos(playerPaddles[1].get_y_pos()+playerPaddles[1].get_speed());
        // std::cout << "is now " << playerPaddles[1].get_y_pos() << std::endl;
    }

    double ballx = ball.get_x_pos();

    // Second, we need to actually check where the ball is
    if (ballx < 0)
    {
        p2Points++;
        ball.set_x_pos(300);
        ball.set_y_pos(200);
        ball.set_direction(-160);
        playerPaddles[0].set_y_pos(300);
        playerPaddles[1].set_y_pos(300);
        playHitSound = true;
        return;
    }
    if (ballx > 800)
    {
        p1Points++;
        ball.set_x_pos(300);
        ball.set_y_pos(200);
        ball.set_direction(-20);
        playerPaddles[0].set_y_pos(300);
        playerPaddles[1].set_y_pos(300);
        playHitSound = true;
        return;
    }

    // Check collisions before applying physics
    checkCollisions();

    ballx = ball.get_x_pos();
    const double bally = ball.get_y_pos();
    // Second, we need to actually check where the ball is
    // Saves us updating things unnecessarily
    const double balldirection = ball.get_direction() * M_PI / 180.0f; // in radians
    const int ballspeed = ball.get_speed();
    // Last, calculate ball's movement using trig
    // the speed will act as the hypotenuse and the direction the angle
    // if we have the hypotenuse and the angle, we can use the equation
    // hypotenuse * cos(angle) to get the tangent (x)
    // hypotenuse * sin(angle) to get the opposite (y)
    // the opposite will be our y value
    // the tangent will be our x value
    const double dx = ballspeed * cos(balldirection);
    const double dy = ballspeed * -sin(balldirection);
    ball.set_xy_pos(ballx + dx, bally + dy);

    std::cout << ball.get_direction() << " degrees, dx: " << dx << ", dy: " << dy << std::endl;
}

void Game::checkCollisions()
{
    const int ballX = ball.get_x_pos();
    const int ballY = ball.get_y_pos();
    float rotation = ball.get_direction();

    // get information about the paddles
    const int p1x = playerPaddles[0].get_x_pos();
    const int p1y = playerPaddles[0].get_y_pos();
    const int p1xSize = playerPaddles[0].get_xSize();
    const int p1ySize = playerPaddles[0].get_ySize();

    const int p2x = playerPaddles[1].get_x_pos();
    const int p2y = playerPaddles[1].get_y_pos();
    const int p2xSize = playerPaddles[1].get_xSize();
    const int p2ySize = playerPaddles[1].get_ySize();


    // Reflect ball when hitting top or bottom
    if (ballY - ball.get_radius() <= 0)
    {
        // flip the ball's rotation
        rotation = -rotation;

        // make sure rotation is positive
        if (rotation < -180)
            rotation += 360;

        // set the rotation direction
        ball.set_direction(rotation);

        // set the position to avoid sticking to the wall
        ball.set_y_pos(.0f + static_cast<float>(ball.get_radius()));

        // set flag to play hit sound
        playHitSound = true;
        std::cout << "Detected top wall hit" << std::endl;
    }
    else if (ballY + ball.get_radius() >= 600)
    {
        // flip the ball's rotation
        rotation = -rotation;

        if (rotation < -180)
            rotation += 360;

        // set the rotation direction
        ball.set_direction(rotation);

        // set the position to avoid sticking to the wall
        ball.set_y_pos(600 - ball.get_radius());

        // set flag to play hit sound
        playHitSound = true;
        std::cout << "Detected bottom wall hit" << std::endl;
    }

    // Ball is within left paddle bounds plus radius
    if (
        ballX - ball.get_radius() >= p1x && // Check if ball is within the width of the paddle
        ballX - ball.get_radius() < p1x + p1xSize &&
        ballY >= p1y && // check if the ball is within the height of the paddle
        ballY <= p1y + p1ySize
    )
    {
        // mirror ball's direction when it hits a paddle
        rotation = 180 - rotation;

        // make sure rotation is positive
        while (rotation < -180)
            rotation += 360;

        // set the direction
        ball.set_direction(rotation);

        // reposition ball to avoid sticking to paddle
        ball.set_x_pos(p1x + p1xSize + ball.get_radius());

        // set flag to play hit sound
        playHitSound = true;
        std::cout << "Detected left paddle hit" << std::endl;
    }
    // Ball is within right paddle bounds plus radius
    else if (
        ballX + ball.get_radius() > p2x && // Check if ball is within the width of the paddle
        ballX + ball.get_radius() <= p2x + p2xSize &&
        ballY >= p2y && // check if the ball is within the height of the paddle
        ballY <= p2y + p2ySize
    )
    {
        // mirror ball's direction when it hits a paddle
        rotation = 180 - rotation;

        // make sure rotation is positive
        while (rotation < -180)
            rotation += 360;

        // set the direction
        ball.set_direction(rotation);

        // reposition ball to avoid sticking to paddle
        ball.set_x_pos(p2x - ball.get_radius());

        // set flag to play hit sound
        playHitSound = true;
        std::cout << "Detected right paddle hit" << std::endl;
    }
}

void Game::encodeData(float* outBuffer) const
{
    // get information about the paddles
    outBuffer[0] = playerPaddles[0].get_x_pos();
    outBuffer[1] = playerPaddles[0].get_y_pos();
    outBuffer[2] = playerPaddles[0].get_speed();
    outBuffer[3] = playerPaddles[0].get_xSize();
    outBuffer[4] = playerPaddles[0].get_ySize();
    outBuffer[5] = p1Points;

    outBuffer[6] = playerPaddles[1].get_x_pos();
    outBuffer[7] = playerPaddles[1].get_y_pos();
    outBuffer[8] = playerPaddles[1].get_speed();
    outBuffer[9] = playerPaddles[1].get_xSize();
    outBuffer[10] = playerPaddles[1].get_ySize();
    outBuffer[11] = p2Points;

    outBuffer[12] = ball.get_x_pos();
    outBuffer[13] = ball.get_y_pos();
    outBuffer[14] = ball.get_speed();
    outBuffer[15] = ball.get_radius();
    outBuffer[16] = ball.get_direction();

    // Whether to play the hit sound effect
    outBuffer[17] = playHitSound ? 1 : 0;
}

void Game::decodeData(float* data)
{
    playerPaddles[0].set_x_pos(data[0]);
    playerPaddles[0].set_y_pos(data[1]);
    playerPaddles[0].set_speed(data[2]);
    playerPaddles[0].set_xSize(data[3]);
    playerPaddles[0].set_ySize(data[4]);
    p1Points = data[5];

    playerPaddles[1].set_x_pos(data[6]);
    playerPaddles[1].set_y_pos(data[7]);
    playerPaddles[1].set_speed(data[8]);
    playerPaddles[1].set_xSize(data[9]);
    playerPaddles[1].set_ySize(data[10]);
    p2Points = data[11];

    ball.set_x_pos(data[12]);
    ball.set_y_pos(data[13]);
    ball.set_speed(data[14]);
    ball.set_radius(data[15]);
    ball.set_direction(data[16]);

    // Reflect the ball when hitting the top of bottom
    if (data[17] == 1)
    {
        Mix_PlayChannel(-1, ballHitSound, 0);
    }
}

void Game::printData() const
{
    // get information about the paddles
    const std::string p1x = std::to_string(playerPaddles[0].get_x_pos());
    const std::string p1y = std::to_string(playerPaddles[0].get_y_pos());
    const std::string p1speed = std::to_string(playerPaddles[0].get_speed());
    const std::string p1xSize = std::to_string(playerPaddles[0].get_xSize());
    const std::string p1ySize = std::to_string(playerPaddles[0].get_ySize());
    const std::string p2x = std::to_string(playerPaddles[1].get_x_pos());
    const std::string p2y = std::to_string(playerPaddles[1].get_y_pos());
    const std::string p2speed = std::to_string(playerPaddles[1].get_speed());
    const std::string p2xSize = std::to_string(playerPaddles[1].get_xSize());
    const std::string p2ySize = std::to_string(playerPaddles[1].get_ySize());
    const std::string ballX = std::to_string(ball.get_x_pos());
    const std::string ballY = std::to_string(ball.get_y_pos());
    const std::string ballSpeed = std::to_string(ball.get_speed());
    const std::string ballRadius = std::to_string(ball.get_radius());
    const std::string ballRotation = std::to_string(ball.get_direction());

    std::string jsonTemplate = "{\n"
        "\t\"p1\" : {\n"
        "\t\t\"xpos\" : " + p1x + ",\n"
        "\t\t\"ypos\" : " + p1y + ",\n"
        "\t\t\"speed\" : " + p1speed + ",\n"
        "\t\t\"sizex\" : " + p1xSize + ",\n"
        "\t\t\"sizey\" : " + p1ySize + ",\n"
        "\t\t\"points\" : " + std::to_string(p1Points) + "\n" +
        "\t},\n"
        "\t\"p2\" : {\n"
        "\t\t\"xpos\" : " + p2x + ",\n"
        "\t\t\"ypos\" : " + p2y + ",\n"
        "\t\t\"speed\" : " + p2speed + ",\n"
        "\t\t\"sizex\" : " + p2xSize + ",\n"
        "\t\t\"sizey\" : " + p2ySize + ",\n"
        "\t\t\"points\" : " + std::to_string(p2Points) + "\n" +
        "\t},\n"
        "\t\"ball\" : {\n"
        "\t\t\"xpos\" : " + ballX + ",\n"
        "\t\t\"ypos\" : " + ballY + ",\n"
        "\t\t\"speed\" : " + ballSpeed + ",\n"
        "\t\t\"radius\" : " + ballRadius + ",\n"
        "\t\t\"direction\" : " + ballRotation + + "\n" +
        "\t}\n"
        "}\n";

    std::cout << jsonTemplate << std::endl;
}
