#include "Client.h"
#include "Server.h"

int main(int argc, char* argv[])
{
    if (argc > 1 && !strcmp(argv[1], "server"))
    {
        // Server here
        Server("127.0.0.1", 1111);
    }
    else
    {
        // Client here
        Client("127.0.0.1", 1111);
    }

    return 0;
}
