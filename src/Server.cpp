//
// Created by Liam on 03/12/2025.
//

#include "Server.h"

// Server2 class implementation

Server::Server(const std::string& ipAddress, const int port)
{
    // This constructor opens a port to listen for incoming connections
    int sockfd = openPort(ipAddress, port);

    if (sockfd > 0) threadListening = true;

    // It the starts listening for incoming connections
    listeningThread = std::thread([this, sockfd]()
    {
        listenForClients(sockfd);
    });

    listeningThread.detach();

    while (threadListening)
    {
        try
        {
            for (int i = 0; i < clients.size(); i++)
            {
                game.playerKeys[2*i] = clients[i]->keys[0];
                game.playerKeys[2*i + 1] = clients[i]->keys[1];
            }
            // std::cout << "Updating keys: "
            //           << game.playerKeys[0] << " " << game.playerKeys[1] << " "
            //           << game.playerKeys[2] << " " << game.playerKeys[3] << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << "Too many clients connected: " << e.what() << std::endl;
        }

        if (clients.size() >= 2)
        {
            game.update();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int Server::openPort(const std::string& ipAddress, const int port)
{
    std::cout << "Hello" << std::endl;

    // First, validate the IP and the Port Number
    if (!(validateIpAddress(ipAddress) && validatePortNumber(port)))
    {
        throw std::invalid_argument("Invalid IP address or port number");
    }

    const std::string str_port = std::to_string(port);

    // setup variables for client connection
    int portno;

    sockaddr_in serv_addr{};

    // Create a socket file descriptor for which to accept incoming connections
    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("Error opening socket");
    }

    // clear the server's address to assign appropriate information
    bzero(&serv_addr, sizeof(serv_addr));

    try
    {
        portno = std::stoi(str_port);
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("Error when trying to convert port number: " + std::string(e.what()));
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server information to the socket file descriptor
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        close(sockfd); // close the socket
        throw std::runtime_error("Error: Could not bind"); // and throw error
    }

    // Start listening for incoming connections
    if (listen(sockfd, 5) < 0) // check for errors
    {
        close(sockfd); // close the socket
        throw std::runtime_error("Error: Listen failed"); // and throw error
    }

    // if everything went well, we now have a port open to listen for connections on
    return sockfd;
}

void Server::listenForClients(const int sockfd)
{
    while (threadListening)
    {
        sockaddr_in cli_addr{};
        socklen_t clilen = sizeof(cli_addr);

        // Accept incoming connection
        const int newsockfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&cli_addr), &clilen);
        if (newsockfd < 0)
        {
            std::cerr << "Error on accept" << std::endl;
            continue; // CONTINUE BREAKS THIS LOOP ONLY AND STARTS IT AGAIN
        }
        std::cout << "New client ID: " << newsockfd << std::endl;

        // if we already have 2 clients connected, reject the new connection
        if (clients.size() == 2)
        {
            close(newsockfd);
        }

        {
            std::lock_guard lock(clientsMutex);
            // lock the clients mutex to protect the clients list while we add a new client

            int clientID = static_cast<int>(clients.size()) + 1;
            // We need to make a reference to add to the clients list
            auto newClient = std::make_shared<ServerClientInstance>(newsockfd, clientID, &game, &gameMutex);
            // Lock the clients mutex to protect the clients list
            clients.push_back(newClient);
            // This should now have added the client to the list safely
        }
        disconnectInactiveClients();
    }
    close(sockfd);
}

bool Server::validateIpAddress(const std::string& ipAddress)
{
    sockaddr_in sa;
    const int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool Server::validatePortNumber(const int& portNumber)
{
    return portNumber > 0 && portNumber < 65535;
}

// Client class implementation

ServerClientInstance::ServerClientInstance(int sockfd, int clientID, Game* gameptr, std::mutex* gameMutexPtr)
{
    _clientID = clientID;
    _sockfd = sockfd;
    game = gameptr;
    gameMutex = gameMutexPtr;
    initialiseThreads();
}

void ServerClientInstance::initialiseThreads()
{
    if (!initialiseConnection())
    {
        std::cerr << "Error initialising connection for client ID " << _clientID << std::endl;
        connected = false;
        return;
    }

    listeningThread = std::thread([this]()
    {
       listenToClient();
    });
    listeningThread.detach();

    sendingThread = std::thread([this]
    {
       sendToClient();
    });
    sendingThread.detach();
}

bool ServerClientInstance::initialiseConnection()
{
    bool assigned = false;
    // try to assign an ID to the client
    while (!assigned)
    {
        if (!trySendStringToClient(std::to_string(_clientID)))
        {
            connected = false;
            return false;
        }

        std::string response = tryListenToClient();
        // std::cout << "Got response from client " << _clientID << ": " << response << std::endl;
        if (response == "ERROR")
        {
            connected = false;
            return false;
        }
        if (response.contains("OK"))
        {
            assigned = true;
        }

        if (!trySendStringToClient("ACK"))
        {
            connected = false;
            return false;
        }
    }

    return true;
}

void ServerClientInstance::listenToClient()
{
    // Get inputs from the client
    std::string response;
    while (connected)
    {
        bzero(response.data(), response.length());
        response = tryListenToClient();

        decodeData(response);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

std::string ServerClientInstance::tryListenToClient()
{
    char buffer[256];
    bzero(buffer, sizeof(buffer));

    const int n = recv(_sockfd, &buffer, sizeof(buffer), 0);

    if (n < 0)
    {
        std::cout << "Error writing to socket" << std::endl;
        close(_sockfd);
        connected = false;
        return "ERROR";
    }

    auto buffer_str = std::string(buffer, static_cast<size_t>(n));
    return buffer_str;
}

void ServerClientInstance::sendToClient()
{
    while (connected)
    {

        constexpr size_t totalBytes = sizeof(int) * 18;
        int data[18];

        // we can use this to scope the lock
        {
            std::lock_guard lock(*gameMutex);
            game->encodeData(data);
        }

        const auto data_char = reinterpret_cast<const char*>(data);
        size_t totalSent = 0;

        // std::cout << "Sending data to client " << _clientID << ": ";
        // for (int i = 0; i < 17; ++i)
        // {
        //     if (i) std::cout << ", ";
        //     std::cout << data[i];
        // }
        // std::cout << std::endl;

        while (totalSent < totalBytes)
        {
            ssize_t sent = send(_sockfd, data_char + totalSent, totalBytes - totalSent, MSG_NOSIGNAL);
            if (sent <= 0)
            {
                if (sent == -1 && errno == EINTR) continue;
                std::cerr << "writeIntToConnection: send failed: " << strerror(errno) << std::endl;

                connectionRetries++;

                if (connectionRetries > 10)
                {
                    close(_sockfd);
                    connected = false;
                }
            }
            totalSent += static_cast<size_t>(sent);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

bool ServerClientInstance::trySendStringToClient(const std::string& message) const
{
    if (const int n = write(_sockfd, &message, sizeof(message)); n < 0)
    {
        std::cout << "Error writing to socket" << std::endl;
        close(_sockfd);
        return false;
    }
    return true;
}

void ServerClientInstance::decodeData(const std::string& data)
{
    if (data.size() >= 2)
    {
        keys[0] = (data[0] == '1');
        keys[1] = (data[1] == '1');
    }
}

void Server::disconnectInactiveClients()
{
    // make sure to lock the clients mutex before modifying the clients list
    std::lock_guard lock(clientsMutex);

    // iterate through the clients and remove any that are disconnected
    for (const auto& client : clients)
    {
        if (client->connected == false)
        {
            std::erase(clients, client);
        }
    }
}


