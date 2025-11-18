#include "Application.h"

#include <iostream>

int main() {
    MetaImGUI::Application app;

    std::cout << "Initializing MetaImGUI..." << std::endl;

    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }

    std::cout << "Running MetaImGUI..." << std::endl;
    app.Run();

    std::cout << "Shutting down..." << std::endl;
    app.Shutdown();

    return 0;
}