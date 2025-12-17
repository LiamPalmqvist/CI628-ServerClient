#include "Client.h"

Client::Client(const std::string& ipAddress, const int port)
{
    // variables are defined here for clarity
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    // the hostent structure is defined as follows:
    // struct hostent {
    // 		char	*h_name;	/* offifial name of host */
    // 		char	**h_aliases;	/* alias list */
    // 		int	h_addrtype;	/* host address type */
    // 		int	h_length;	/* lenfth of address */
    // 		char	**h_addr_list;	/* list of addresses from name server */
    // 	#define h_addr 	h_addr_list[0]  /* address, for backward compatibility */
    // }
    // it defines a host computer on the Internet. The members of this structure are
    // h_name		Official name of the host.
    // h_aliases	A zero terminated array of alternate names for the host
    // h_addrtype	The type of address being returned; currently always AF_INET
    // h_length		The length, in bytes, of the address
    // h_addr_list	A pointer to a list of network addresses for the names host. Host addresses are returned in the network byte order.
    // NOTE: h_addr is an alias for the first address in the array of network addresses

    // Validate port number and ip address
    if (!(validateIpAddress(ipAddress) && this->validatePortNumber(port)))
    {
        std::cout << "ERROR port number or ip wrong format" << std::endl;
        return;
    }
    std::string str_port = std::to_string(port);

    portno = std::atoi(str_port.data());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cout << "Error creating socket" << std::endl;
        return;
    }

    // This function is included in the <netdb.h> library and allows the user to get an IP Address of a host name
    // if an IP address is supplied, no lookup is performed and it is copied into the h_name field
    server = gethostbyname(ipAddress.data());
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr,
          (char*)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    // This code sets the fields in serv_addr. Much of it is the same as in the server. However, because the field server->h_addr is a character string we use the function:
    // void bcopy(char *s1, char *s2, int length) which copies length bytes from s1 to s2

    if (connect(sockfd, reinterpret_cast<const struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        std::cout << "Error connecting" << std::endl;
        return;
    }

    connected = true;

    listeningThread = std::thread(&Client::listenToServer, this, sockfd);
    sendingThread = std::thread(&Client::sendToServer, this, sockfd);

    init_SDL(sockfd);

    if (!windowIsOpen)
    {
        listeningThread.detach();
        sendingThread.detach();
        return;
    }
}

void Client::listenToServer(const int sockfd)
{
    std::string buffer_str;
    int* buffer_int;
    while (connected)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1/60));
        if (!assigned)
        {
            // listen for ID assignment
            std::cout << "Trying to assign to Client ID" << std::endl;
            buffer_str = tryRecvStringFromServer(sockfd);
            std::cout << "Client ID: " << buffer_str << std::endl;
            if (buffer_str.length() != 0)
            {
                std::cout << buffer_str.length() << std::endl;
                try
                {
                    clientID = std::stoi(buffer_str);
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                    std::cout << "Trying again" << std::endl;
                    tryWriteToServer(sockfd, "ERROR");
                    continue;
                }
            }

            //assigned = true;

            // Send ID assignment back
            std::cout << "Didn't break" << std::endl;
            tryWriteToServer(sockfd, "OK");
            std::cout << "Sending Client ID back to server" << std::endl;

            bzero(buffer_str.data(), buffer_str.length());

            // Listen for ACK
            buffer_str = tryRecvStringFromServer(sockfd);
            if (buffer_str.contains("ACK"))
            {
                assigned = true;
            }

            bzero(buffer_str.data(), buffer_str.length());

        }
        else
        {
            // std::cout << "Trying to recieve int array from server" << std::endl;
            buffer_int = tryRecvIntFromServer(sockfd);
            game.decodeData(buffer_int);
            // game.printData();
            bzero(buffer_int, sizeof(int)*17);
            // We can do this because we know the size of the data coming over
            // exactly
        }
    }
}

// Disable with comment
std::string Client::tryRecvStringFromServer(const int sockfd)
{
    char buffer[256];

    if (const int n = recv(sockfd, buffer, sizeof(buffer), 0); n <= 0)
    {
        if (n == 0)
        {
            std::cout << "Client disconnected" << std::endl;
        }
        else
        {
            std::cout << "Error reading from socket" << std::endl;
        }
        connected = false;
        close(sockfd);
    }

    return buffer;
}

// Disable with comment
int* Client::tryRecvIntFromServer(const int sockfd)
{
    //std::cout << "Trying to read from socket" << std::endl;

    static int buffer[18];
    if (const int n = recv(sockfd, buffer, sizeof(buffer), 0); n <= 0)
    {
        if (n == 0)
        {
            std::cout << "Client disconnected" << std::endl;
        }
        else
        {
            std::cout << "Error reading from socket" << std::endl;
        }
        connected = false;
        close(sockfd);
    }

    // std::cout << "BUFFER: " << std::endl;
    // std::string str = "";
    // int i = 0;
    // for (i; i < 16; i++)
    // {
    //     str += std::to_string(buffer[i]) + ", ";
    // }
    // str += std::to_string(buffer[i+1]);
    // std::cout << str << std::endl;
    // std::cout << "END BUFFER" << std::endl;

    return buffer;
}

void Client::sendToServer(int sockfd)
{
    //char buffer[256];
    std::string message;
    while (connected)
    {
        if (assigned)
        {
            message = keys[0] == true ? "1" : "0";
            message += (keys[1] == true ? "1" : "0");
            //std::cout << "Message to server: " << message << std::endl;
            tryWriteToServer(sockfd, message);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        //if (game.playing)
        //{
        //}
    }
    // while (connected)
    // {
    //     fgets(buffer, sizeof(buffer), stdin);
    //     message[strcspn(buffer, "\n")] = 0;
    //     tryWriteToServer(sockfd, message);
    // }
}

// Disable warning
void Client::tryWriteToServer(const int sockfd, const std::string& message)
{
    if (const int n = write(sockfd, &message, sizeof(message)); n < 0)
    {
        std::cout << "Error writing to socket" << std::endl;
        close(sockfd);
        connected = false;
    }
    //std::cout << "Wrote " << message << " to server" << std::endl;
}

bool Client::validateIpAddress(const std::string& ipAddress)
{
    struct sockaddr_in sa;
    const int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool Client::validatePortNumber(const int& portNumber)
{
    return (portNumber > 0 && portNumber < 65535);
}

void Client::init_SDL(const int sockfd) {
    // Check if we can create an SDL Window
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Start creating a window for SDL
    window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );

    // if the window creation failed, print the error and exit
    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        windowIsOpen = false;
        return;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    // This function creates a renderer for the window, using hardware acceleration
    // and VSync
    // -1 indicates that it will use the first available rendering driver

    // If the renderer creation fails, print the error and exit
    if (!renderer) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError();
        SDL_DestroyWindow(window);
        SDL_Quit();
        windowIsOpen = false;
        return;
    }

    // Move the threading here
    // listeningThread = std::thread(&Client::listenToServer, this, sockfd);
    // sendingThread = std::thread(&Client::sendToServer, this, sockfd);

    // Instantiate SDL_Mixer here
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout << "SDL_Mixer could not initialize! SDL_Mixer Error: " << Mix_GetError() << std::endl;
        windowIsOpen = false;
        return;
    }

    // Finally, we get to the main loop
    SDL_Event event;

    // Game game;
    loadMedia();
    instantiateGameObjects();

    // std::cout << "SDL_Event " << event.type << std::endl;

    while (windowIsOpen)
    {
        //std::cout << "Window stuff happening" << std::endl;
        // Process events
        while (SDL_PollEvent(&event)) {
            getInputs(event);
            if (event.type == SDL_QUIT) {
                windowIsOpen = false;
            }
        }

        // Updating positions of objects goes here

        updateGameObjects();

        SDL_RenderClear(renderer);
        // Render call goes here
        renderGameObjects();

        SDL_RenderPresent(renderer);
    }


    // Free resources and close SDL
    for (int i = 0; i < 10; i++)
    {
        SDL_DestroyTexture(numbers[i]);
        numbers[i] = nullptr;
    }

    Mix_FreeChunk(ballSound);
    ballSound = nullptr;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;

    IMG_Quit();
    Mix_Quit();
    SDL_Quit();

    sendingThread.detach();
    listeningThread.detach();
}

void Client::getInputs(SDL_Event &event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        keys[0] = event.type == SDL_KEYDOWN;
        break;
    case SDLK_s:
        keys[1] = event.type == SDL_KEYDOWN;
        break;
    default:
        break;
    }
    //std::cout << "keys: " << keys[0] << " " << keys[1] << std::endl;
}

void Client::loadMedia()
{
    for (int i = 0; i < 10; i++)
    {
        std::string path = "assets/numbers/" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            std::cout << "IMG_Load Error: " << IMG_GetError() << std::endl;
            continue;
        }
        numbers[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!numbers[i])
        {
            std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        }
    }

    game.ballHitSound = Mix_LoadWAV("assets/sounds/ball_hit.wav");
    if (game.ballHitSound == NULL)
    {
        std::cout << "Failed to load ball hit sound effect! SDL_Mixer Error: " << Mix_GetError() << std::endl;
        windowIsOpen = false;
    }
}

void Client::instantiateGameObjects()
{
    for (auto paddle : game.playerPaddles)
    {
        SDL_Rect paddleRect = {paddle.get_x_pos(), paddle.get_y_pos(), paddle.get_xSize(), paddle.get_ySize()};
        playerPaddles.push_back(paddleRect);
    }
}

void Client::renderGameObjects() const
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (auto paddle : playerPaddles)
    {
        SDL_RenderFillRect(renderer, &paddle);
    }

    // Draw the images for the scores
    int p1score = game.p1Points;
    int p2score = game.p2Points;
    SDL_Rect destRect1 = {350, 50, 50, 50};
    SDL_Rect destRect2 = {400, 50, 50, 50};
    SDL_RenderCopy(renderer, numbers[p1score], nullptr, &destRect1);
    SDL_RenderCopy(renderer, numbers[p2score], nullptr, &destRect2);

    const int centerX = game.ball.get_x_pos();
    const int centerY = game.ball.get_y_pos();
    const int radius = game.ball.get_radius();

    for (int i = 0; i < 180; i++)
    {
        float angle = i * M_PI / 180.0f;

        float startx = centerX - cos(angle) * radius;
        float starty = centerY - sin(angle) * radius;
        float endx = centerX + cos(angle) * radius;
        float endy = centerY + sin(angle) * radius;

        SDL_RenderDrawLine(renderer, startx, starty, endx, endy);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    float angle = game.ball.get_direction() * M_PI / 180.0f;
    float endx = centerX + cos(angle) * radius * 2;
    float endy = centerY + sin(angle) * radius * 2;
    SDL_RenderDrawLine(renderer, centerX, centerY, endx, endy);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


    //SDL_RenderDrawLine(renderer, 0, 300, 800, 300);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}

void Client::updateGameObjects()
{
    for (int i = 0; i < playerPaddles.size(); i++)
    {
        playerPaddles[i].x = game.playerPaddles[i].get_x_pos();
        playerPaddles[i].y = game.playerPaddles[i].get_y_pos();
        playerPaddles[i].w = game.playerPaddles[i].get_xSize();
        playerPaddles[i].h = game.playerPaddles[i].get_ySize();
    }
}