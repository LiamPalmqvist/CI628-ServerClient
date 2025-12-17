#pragma once

#include <sys/socket.h>
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h>
#include <netdb.h> // for gethostbyname
#include <iostream>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDl2_mixer/SDL_mixer.h>

#include "Game.h"

class Client {
    // Private variables
    bool connected;
    std::thread listeningThread;
    std::thread sendingThread;
    bool assigned = false;
    int clientID{};

    Game game;

    // SDL INFORMATION
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    bool windowIsOpen = true;
    bool keys[2] = {false, false};

    // SDL_Image INFORMATION
    SDL_Texture* numbers[10] = {};

    // SDL_Mixer INFORMATION
    Mix_Chunk* ballSound = nullptr;

    std::vector<SDL_Rect> playerPaddles;

    // SDL FUNCTIONS
    void init_SDL(const int sockfd);
    void getInputs(SDL_Event &event);
    void loadMedia();
    void instantiateGameObjects();
    void renderGameObjects() const;
    void updateGameObjects();

    // Private functions
    static bool validateIpAddress(const std::string &ipAddress);
    static bool validatePortNumber(const int &portNumber);
    std::string tryRecvStringFromServer(int sockfd);
    int* tryRecvIntFromServer(int sockfd);
    void tryWriteToServer(int sockfd, const std::string& message);
    void listenToServer(int sockfd);
    void sendToServer(int sockfd);
public:
    // Public constructor
    Client(const std::string& ipAddress, int port);
};
