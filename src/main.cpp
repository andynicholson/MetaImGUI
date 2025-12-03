#include "Application.h"
#include "Logger.h"

int main() {
    MetaImGUI::Application app;

    LOG_INFO("Initializing MetaImGUI...");

    if (!app.Initialize()) {
        LOG_ERROR("Failed to initialize application!");
        return -1;
    }

    LOG_INFO("Running MetaImGUI...");
    app.Run();

    LOG_INFO("Shutting down...");
    app.Shutdown();

    return 0;
}
