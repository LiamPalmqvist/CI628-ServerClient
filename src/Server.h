//
// Created by Liam on 03/12/2025.
//

#pragma once
#ifndef CLIENT_SERVER2_H
#define CLIENT_SERVER2_H
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Game.h"

class ServerClientInstance
{
    int _clientID;

    Game* game;
    std::mutex* gameMutex;

    std::thread listeningThread;
    std::thread sendingThread;

    // each client needs its own socket file descriptor
    int _sockfd;
    int connectionRetries = 0;

    void resetConnectionRetries();

    // the class also needs to listen to incoming data from the client
    // as well as send data to the client
    // This is done inside the Client class as it means
    // that each client can its own connection
    void initialiseThreads();
    bool initialiseConnection();

    // broad sending and receiving functions
    void listenToClient();
    void sendToClient();

    // try functions that return bool on success/failure
    std::string tryListenToClient();
    bool trySendStringToClient(const std::string& message) const;

    void decodeData(const std::string& data);

public:
    // pass the clients vector by reference to avoid copying and to
    // keep the data up to date
    ServerClientInstance(int sockfd, int clientID, Game* gameptr, std::mutex* gameMutexPtr);
    bool keys[2] = {false, false};
    bool connected = true;
};

class Server
{
    Game game;
    std::mutex gameMutex;
    // Since the clients need to be mutable by multiple threads,
    // we need to protect the client list with a mutex
    std::vector<std::shared_ptr<ServerClientInstance>> clients;
    std::mutex clientsMutex;

    bool threadListening = false;
    std::thread listeningThread;

    static bool validateIpAddress(const std::string& ipAddress);
    static bool validatePortNumber(const int& portNumber);

    static int openPort(const std::string& ipAddress, int port);
    void listenForClients(int sockfd);
    void disconnectInactiveClients();

public:
    // IP Address is passed as reference because
    // it is not modified, and we want to avoid copying
    // Port is passed by value as it is a primitive type
    Server(const std::string& ipAddress, int port);
};


#endif //CLIENT_SERVER2_H