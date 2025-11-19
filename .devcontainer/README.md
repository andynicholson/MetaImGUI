# MetaImGUI Development Container

This directory contains the VS Code devcontainer configuration for MetaImGUI development.

## What is a Devcontainer?

A development container (devcontainer) is a Docker container configured specifically for development. It provides:

- **Consistent environment** across all developers
- **Pre-installed tools** and dependencies
- **Isolated development** from your host system
- **Quick onboarding** for new contributors

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed and running
- [Visual Studio Code](https://code.visualstudio.com/)
- [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## Quick Start

1. Open VS Code
2. Open the MetaImGUI project folder
3. When prompted, click "Reopen in Container" (or press F1 and select "Dev Containers: Reopen in Container")
4. Wait for the container to build (first time only, takes 5-10 minutes)
5. The environment is ready when you see the terminal prompt

## What's Included

### Build Tools
- GCC 12
- Clang 15
- CMake 3.22+
- Ninja build system
- ccache (for faster rebuilds)

### Development Tools
- clangd (language server)
- clang-format (code formatting)
- clang-tidy (static analysis)
- GDB and LLDB (debuggers)

### Testing & Quality
- lcov & gcovr (code coverage)
- Google Benchmark (performance testing)
- Address/Thread/UB Sanitizers

### Documentation
- Doxygen (API documentation)
- Graphviz (diagram generation)

### VS Code Extensions
Pre-installed extensions:
- C/C++ Tools
- CMake Tools
- clangd
- GitLens
- GitHub Copilot (if you have it)

## Usage

### Building

```bash
# Configure (already done by post-create script)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel

# Or use VS Code CMake Tools (Ctrl+Shift+P -> CMake: Build)
```

### Testing

```bash
# Run all tests
cd build && ctest --output-on-failure

# Or press Ctrl+Shift+P -> CMake: Run Tests
```

### Code Coverage

```bash
./scripts/run_coverage.sh
```

### Sanitizers

```bash
./scripts/run_sanitizers.sh ALL
```

### Formatting

```bash
# Format all code
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Or save files in VS Code (format on save is enabled)
```

## GUI Applications

The devcontainer is configured to forward X11 to your host machine (Linux only). To run the GUI application:

```bash
./build/MetaImGUI
```

### macOS & Windows

For macOS and Windows, you'll need to set up X11 forwarding separately:

**macOS:**
1. Install [XQuartz](https://www.xquartz.org/)
2. Run: `xhost +localhost`
3. Set `DISPLAY` in the container: `export DISPLAY=host.docker.internal:0`

**Windows:**
1. Install [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
2. Run XLaunch with "Disable access control" checked
3. Set `DISPLAY` in the container: `export DISPLAY=host.docker.internal:0.0`

## Customization

### Adding Packages

Edit `.devcontainer/Dockerfile` and rebuild:
```dockerfile
RUN apt-get update && apt-get install -y \
    your-package-here
```

Then rebuild the container:
- Press F1
- Select "Dev Containers: Rebuild Container"

### VS Code Settings

Edit `.devcontainer/devcontainer.json` to modify VS Code settings or add extensions.

## Troubleshooting

### Container Build Fails

```bash
# Clean Docker cache and rebuild
docker system prune -a
# Then rebuild in VS Code
```

### GUI Not Working

```bash
# Check X11 forwarding
echo $DISPLAY
xeyes  # Should show a window

# Allow X11 connections (Linux host)
xhost +local:docker
```

### Slow Builds

The container uses ccache by default. After the first build, subsequent builds should be much faster.

```bash
# Check ccache stats
ccache -s
```

### Permission Issues

```bash
# Fix file ownership if needed
sudo chown -R $USER:$USER /workspace
```

## Performance Tips

1. **Use named volumes** for better I/O performance
2. **Enable Docker resource limits** if memory-constrained
3. **Use ccache** (already configured)
4. **Build with Ninja** (already configured)

## Development Workflow

1. Make changes in VS Code
2. Code is automatically formatted on save
3. Build with `Ctrl+Shift+B` or CMake Tools
4. Run tests with CMake Tools test button
5. Commit with pre-commit hook (auto-formatting)

## Additional Resources

- [VS Code Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)
- [Docker Documentation](https://docs.docker.com/)
- [MetaImGUI Documentation](../README.md)

## Support

If you encounter issues:
1. Check the troubleshooting section above
2. Review container logs: View -> Output -> Dev Containers
3. Open an issue on GitHub

