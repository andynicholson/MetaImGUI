# File Dialog Integration Guide

MetaImGUI doesn't include file dialogs by default to keep dependencies minimal. However, you can easily add cross-platform file dialog support using one of these libraries:

## Recommended Libraries

### 1. Portable File Dialogs (Recommended)

**Repository**: https://github.com/samhocevar/portable-file-dialogs

**Advantages**:
- Header-only library (no build required)
- Cross-platform (Windows, Linux, macOS)
- Native dialogs on all platforms
- No external dependencies

**Installation**:
```bash
cd external
git clone https://github.com/samhocevar/portable-file-dialogs.git pfd
```

**Usage Example**:
```cpp
#include <portable-file-dialogs.h>

// Open file dialog
auto selection = pfd::open_file("Select a file").result();
if (!selection.empty()) {
    std::string filepath = selection[0];
    // Load file...
}

// Save file dialog
auto destination = pfd::save_file("Save file").result();
if (!destination.empty()) {
    // Save to destination...
}

// Select folder
auto folder = pfd::select_folder("Select folder").result();
if (!folder.empty()) {
    // Use folder...
}
```

### 2. Native File Dialog Extended

**Repository**: https://github.com/btzy/nativefiledialog-extended

**Advantages**:
- Native OS dialogs
- Filter support
- Multiple selection
- Good performance

**Installation**:
```bash
cd external
git clone https://github.com/btzy/nativefiledialog-extended.git nfd
```

**CMake Integration**:
```cmake
add_subdirectory(external/nfd)
target_link_libraries(MetaImGUI PRIVATE nfd)
```

**Usage Example**:
```cpp
#include <nfd.hpp>

// Open file
NFD::UniquePath outPath;
nfdfilteritem_t filters[2] = { { "Source code", "cpp,h" }, { "Text files", "txt" } };
nfdresult_t result = NFD::OpenDialog(outPath, filters, 2);

if (result == NFD_OKAY) {
    std::cout << "Selected: " << outPath.get() << std::endl;
}

// Save file
result = NFD::SaveDialog(outPath, filters, 2);
```

### 3. ImGui File Dialog (ImGuiFileDialog)

**Repository**: https://github.com/aiekick/ImGuiFileDialog

**Advantages**:
- Pure ImGui implementation
- No external dependencies
- Customizable
- Consistent look across platforms

**Installation**:
```bash
cd external
git clone https://github.com/aiekick/ImGuiFileDialog.git
```

**Usage Example**:
```cpp
#include <ImGuiFileDialog.h>

// Open dialog
if (ImGui::Button("Open File")) {
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h", ".");
}

// Display and handle result
if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        // Use selected file...
    }
    ImGuiFileDialog::Instance()->Close();
}
```

## Integration Steps

### 1. Choose a Library

Select the library that best fits your needs:
- **Portable File Dialogs**: Best for native dialogs with minimal setup
- **NFD Extended**: Best for advanced filtering and native integration
- **ImGuiFileDialog**: Best for consistent ImGui-styled dialogs

### 2. Add to CMakeLists.txt

```cmake
# Example for Portable File Dialogs (header-only)
target_include_directories(MetaImGUI PRIVATE
    external/pfd
)

# Example for NFD Extended
add_subdirectory(external/nfd)
target_link_libraries(MetaImGUI PRIVATE nfd)

# Example for ImGuiFileDialog
add_subdirectory(external/ImGuiFileDialog)
target_link_libraries(MetaImGUI PRIVATE ImGuiFileDialog)
```

### 3. Create Wrapper Class

Create a `FileDialog` class to abstract the implementation:

```cpp
// include/FileDialog.h
#pragma once
#include <string>
#include <vector>

namespace MetaImGUI {

struct FileDialogFilter {
    std::string name;
    std::string pattern;
};

class FileDialog {
public:
    static std::string OpenFile(
        const std::string& title = "Open File",
        const std::vector<FileDialogFilter>& filters = {}
    );

    static std::string SaveFile(
        const std::string& title = "Save File",
        const std::vector<FileDialogFilter>& filters = {}
    );

    static std::string SelectFolder(
        const std::string& title = "Select Folder"
    );

    static std::vector<std::string> OpenMultipleFiles(
        const std::string& title = "Open Files",
        const std::vector<FileDialogFilter>& filters = {}
    );
};

} // namespace MetaImGUI
```

### 4. Update Documentation

Update your README.md to mention file dialog support and which library you chose.

## Platform Considerations

### Windows
- Native file dialogs use Windows Explorer integration
- Supports recent files and favorites
- May require COM initialization

### Linux
- Uses GTK or Qt dialogs depending on system
- May require GTK development libraries
- Falls back to zenity/kdialog if available

### macOS
- Uses Cocoa native dialogs
- Supports Finder integration
- Requires Objective-C++ for some implementations

## Testing

Test file dialogs on each platform:
```cpp
// Test cases
void TestFileDialogs() {
    // Test open
    auto file = FileDialog::OpenFile("Test Open");
    if (!file.empty()) {
        LOG_INFO("Selected: {}", file);
    }

    // Test save
    auto save = FileDialog::SaveFile("Test Save");
    if (!save.empty()) {
        LOG_INFO("Save to: {}", save);
    }

    // Test folder
    auto folder = FileDialog::SelectFolder("Test Folder");
    if (!folder.empty()) {
        LOG_INFO("Folder: {}", folder);
    }
}
```

## References

- [Portable File Dialogs](https://github.com/samhocevar/portable-file-dialogs)
- [NFD Extended](https://github.com/btzy/nativefiledialog-extended)
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
- [File Dialogs on Wikipedia](https://en.wikipedia.org/wiki/File_dialog)

