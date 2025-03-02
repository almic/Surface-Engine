#include "app/app.h"

#include <iostream>

class SandboxApp : public Surface::App
{
    void update()
    {
        std::cout << "Calling SandboxApp update()" << std::endl;

        stop(true);
    };
};

int main()
{
    std::cout << "Test" << std::endl;

    SandboxApp app;

    int result = app.run();

    std::cout << "Stopped on tick " << app.tick_count() << std::endl;
    return result;
}
