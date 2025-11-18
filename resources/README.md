# Resources Directory

This directory is for application resources such as:

- Images and textures
- Configuration files
- Data files
- Fonts
- Icons

The build system automatically copies this directory to the build output, so resources can be loaded at runtime using relative paths from the executable.

## Usage

Place your resource files here and they will be available in the build directory. Access them in your code like this:

```cpp
// Example: loading a configuration file
std::ifstream configFile("resources/config.json");
```

## Structure

You can organize resources in subdirectories as needed:

```
resources/
├── images/
├── config/
├── fonts/
└── data/
``` 