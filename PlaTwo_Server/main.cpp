#include <iostream>
#include "GameServer.h"

int main()
{
    GameServer server(12345);

    if (!server.start()) {
        std::cerr << "خطا در اجرای سرور!" << std::endl;
        return -1;
    }

    return 0;
}
