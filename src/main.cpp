#include "Application.h"

#include <iostream>

int main() {
    MetaImGUI::Application app;

    std::cout << "Initializing MetaImGUI...\n";

    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application!\n";
        return -1;
    }

    std::cout << "Running MetaImGUI...\n";
    app.Run();

    std::cout << "Shutting down...\n";
    app.Shutdown();

    return 0;
}
