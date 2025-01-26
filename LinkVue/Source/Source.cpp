#include "Application.h"

int main()
{
    Application app;

    if (!app.init()) {
        std::cerr << "Failed to initialize the application." << std::endl;
        return -1;
    }

    app.Run();

    return 0; 
}