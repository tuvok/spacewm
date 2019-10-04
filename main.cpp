#include <WindowManager.h>

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Starting Window Manager" << std::endl;

    auto wm = WindowManager::create();

    wm->run();

    return 0;
}
