# Plugin Architecture Design

This document outlines how to implement a plugin/extension system for MetaImGUI applications.

## Overview

A plugin architecture allows users to extend your application without modifying the core code. Plugins can add:
- New UI panels/windows
- Custom menu items
- Data processors
- Import/export formats
- Themes and visualizations

## Architecture Design

### 1. Plugin Interface

Define a base plugin interface that all plugins must implement:

```cpp
// include/IPlugin.h
#pragma once
#include <string>
#include <memory>

namespace MetaImGUI {

class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Plugin metadata
    virtual std::string GetName() const = 0;
    virtual std::string GetVersion() const = 0;
    virtual std::string GetAuthor() const = 0;
    virtual std::string GetDescription() const = 0;

    // Lifecycle
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // Called each frame if plugin is active
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    // Menu integration
    virtual void RenderMenu() {}

    // Optional settings
    virtual void RenderSettings() {}
};

} // namespace MetaImGUI
```

### 2. Plugin Manager

Create a manager to load, unload, and manage plugins:

```cpp
// include/PluginManager.h
#pragma once
#include "IPlugin.h"
#include <vector>
#include <map>
#include <string>
#include <filesystem>

namespace MetaImGUI {

class PluginManager {
public:
    static PluginManager& Instance();

    // Plugin loading
    bool LoadPlugin(const std::filesystem::path& path);
    bool LoadPluginsFromDirectory(const std::filesystem::path& dir);
    void UnloadPlugin(const std::string& name);
    void UnloadAllPlugins();

    // Plugin access
    IPlugin* GetPlugin(const std::string& name);
    std::vector<IPlugin*> GetAllPlugins();
    std::vector<std::string> GetPluginNames() const;

    // Plugin lifecycle
    void InitializeAll();
    void ShutdownAll();
    void UpdateAll(float deltaTime);
    void RenderAll();

    // Enable/disable
    void EnablePlugin(const std::string& name, bool enable);
    bool IsPluginEnabled(const std::string& name) const;

private:
    PluginManager() = default;
    ~PluginManager();

    struct PluginInfo {
        std::unique_ptr<IPlugin> plugin;
        void* handle; // DLL/SO handle
        bool enabled;
    };

    std::map<std::string, PluginInfo> m_plugins;
};

} // namespace MetaImGUI
```

### 3. Dynamic Loading

Plugins can be loaded at runtime using platform-specific APIs:

#### Windows (DLL)
```cpp
#include <windows.h>

void* LoadPluginLibrary(const char* path) {
    return LoadLibraryA(path);
}

void* GetPluginFunction(void* handle, const char* name) {
    return GetProcAddress((HMODULE)handle, name);
}

void UnloadPluginLibrary(void* handle) {
    FreeLibrary((HMODULE)handle);
}
```

#### Linux/macOS (SO/dylib)
```cpp
#include <dlfcn.h>

void* LoadPluginLibrary(const char* path) {
    return dlopen(path, RTLD_LAZY);
}

void* GetPluginFunction(void* handle, const char* name) {
    return dlsym(handle, name);
}

void UnloadPluginLibrary(void* handle) {
    dlclose(handle);
}
```

### 4. Plugin Export

Each plugin must export a factory function:

```cpp
// Example plugin implementation
// MyPlugin.cpp

#include "IPlugin.h"

class MyPlugin : public MetaImGUI::IPlugin {
public:
    std::string GetName() const override { return "My Plugin"; }
    std::string GetVersion() const override { return "1.0.0"; }
    std::string GetAuthor() const override { return "Author Name"; }
    std::string GetDescription() const override { return "Does something cool"; }

    bool Initialize() override {
        LOG_INFO("MyPlugin initialized");
        return true;
    }

    void Shutdown() override {
        LOG_INFO("MyPlugin shutdown");
    }

    void Update(float deltaTime) override {
        // Update plugin state
    }

    void Render() override {
        ImGui::Begin("My Plugin Window");
        ImGui::Text("Hello from plugin!");
        ImGui::End();
    }

    void RenderMenu() override {
        if (ImGui::BeginMenu("My Plugin")) {
            if (ImGui::MenuItem("Do Something")) {
                // Handle action
            }
            ImGui::EndMenu();
        }
    }
};

// Export function
extern "C" {
    #ifdef _WIN32
    __declspec(dllexport)
    #endif
    MetaImGUI::IPlugin* CreatePlugin() {
        return new MyPlugin();
    }

    #ifdef _WIN32
    __declspec(dllexport)
    #endif
    void DestroyPlugin(MetaImGUI::IPlugin* plugin) {
        delete plugin;
    }
}
```

### 5. Plugin Configuration

Store plugin settings in JSON:

```json
// plugins/config.json
{
  "enabled_plugins": [
    "MyPlugin",
    "AnotherPlugin"
  ],
  "plugin_settings": {
    "MyPlugin": {
      "window_visible": true,
      "custom_setting": "value"
    }
  }
}
```

## Implementation Steps

### Step 1: Define Plugin Interface

1. Create `include/IPlugin.h` with base interface
2. Design lifecycle methods (Initialize, Update, Render, Shutdown)
3. Add metadata methods (GetName, GetVersion, etc.)

### Step 2: Create Plugin Manager

1. Implement `PluginManager` singleton
2. Add dynamic library loading
3. Implement plugin discovery
4. Add enable/disable functionality

### Step 3: Integrate with Application

```cpp
// In Application::Initialize()
PluginManager::Instance().LoadPluginsFromDirectory("plugins/");
PluginManager::Instance().InitializeAll();

// In Application::Run() loop
PluginManager::Instance().UpdateAll(deltaTime);
PluginManager::Instance().RenderAll();

// In Application::Shutdown()
PluginManager::Instance().ShutdownAll();
```

### Step 4: Create Example Plugin

1. Set up separate CMake project for plugin
2. Link against main application headers
3. Export factory functions
4. Build as shared library (.dll/.so/.dylib)

### Step 5: Add Plugin UI

```cpp
// In menu bar
if (ImGui::BeginMenu("Plugins")) {
    for (auto& name : PluginManager::Instance().GetPluginNames()) {
        bool enabled = PluginManager::Instance().IsPluginEnabled(name);
        if (ImGui::Checkbox(name.c_str(), &enabled)) {
            PluginManager::Instance().EnablePlugin(name, enabled);
        }
    }
    ImGui::EndMenu();
}
```

## Plugin Distribution

### Directory Structure
```
plugins/
├── myplugin/
│   ├── myplugin.dll (Windows)
│   ├── libmyplugin.so (Linux)
│   ├── libmyplugin.dylib (macOS)
│   ├── config.json
│   ├── resources/
│   └── README.md
```

### Plugin Manifest

Each plugin should include a manifest:

```json
// plugin.json
{
  "name": "My Plugin",
  "version": "1.0.0",
  "author": "Author Name",
  "description": "Plugin description",
  "requires_version": "1.0.0",
  "dependencies": [],
  "platform": "all"
}
```

## Security Considerations

1. **Sandboxing**: Consider isolating plugins from critical system resources
2. **Verification**: Implement plugin signature verification
3. **Permissions**: Define what plugins can access
4. **Loading**: Only load plugins from trusted directories
5. **Error Handling**: Ensure plugin crashes don't crash the host

## Testing

Create test plugins:

```cpp
// Test plugin that exercises all interface methods
class TestPlugin : public IPlugin {
    // Implement all methods with logging
    // Verify callbacks are called correctly
    // Test enable/disable functionality
};
```

## Alternative Approaches

### 1. Scripting (Lua/Python)

Instead of compiled plugins, use embedded scripting:

```cpp
// Lua example
#include <lua.hpp>

class LuaPlugin : public IPlugin {
    lua_State* L;

    bool Initialize() override {
        L = luaL_newstate();
        luaL_openlibs(L);
        luaL_dofile(L, "plugin.lua");
        return true;
    }

    void Update(float dt) override {
        lua_getglobal(L, "update");
        lua_pushnumber(L, dt);
        lua_call(L, 1, 0);
    }
};
```

### 2. Static Plugins

For simplicity, register plugins at compile time:

```cpp
class PluginRegistry {
public:
    template<typename T>
    static void Register(const std::string& name) {
        factories[name] = []() { return std::make_unique<T>(); };
    }

private:
    static std::map<std::string, std::function<std::unique_ptr<IPlugin>()>> factories;
};

// In plugin source
REGISTER_PLUGIN(MyPlugin, "My Plugin");
```

## Resources

- [Plugin Architecture Patterns](https://en.wikipedia.org/wiki/Plug-in_(computing))
- [Dynamic Loading on Windows](https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-functions)
- [Dynamic Loading on Linux](https://man7.org/linux/man-pages/man3/dlopen.3.html)
- [Lua Embedding](https://www.lua.org/manual/5.4/manual.html#4)
- [Python Embedding](https://docs.python.org/3/extending/embedding.html)

## Examples

See the `examples/plugins/` directory for:
- Simple plugin example
- Plugin with UI
- Plugin with settings
- Plugin with menu integration

