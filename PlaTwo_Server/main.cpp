#include <iostream>
#include <string>
#include "GameServer.h"

int main(int argc, char *argv[])
{
    int port = 12345;

    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 12345;
        }
    }

    GameServer server(port);

    if (!server.start()) {
        std::cerr << "\n[!] Khata dar ejraye server rooye port " << port << std::endl;
        std::cerr << "Ehtemalan port dar hale estefade ast ya Firewall an ra block karde." << std::endl;
        std::cin.get();
        return -1;
    }

    return 0;
}
