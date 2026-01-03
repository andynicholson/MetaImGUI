# Contributing to MetaImGUI

First off, thank you for considering contributing to MetaImGUI! It's people like you that make MetaImGUI such a great template for the community.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)

## Code of Conduct

This project and everyone participating in it is governed by the [MetaImGUI Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When you create a bug report, include as many details as possible:

- **Use a clear and descriptive title**
- **Describe the exact steps to reproduce the problem**
- **Provide specific examples** (code snippets, screenshots)
- **Describe the behavior you observed and what you expected**
- **Include details about your configuration and environment**:
  - OS version (Linux distro, Windows version, macOS version)
  - Compiler version (GCC, Clang, MSVC)
  - CMake version
  - ImGui version

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion:

- **Use a clear and descriptive title**
- **Provide a step-by-step description** of the suggested enhancement
- **Provide specific examples** to demonstrate the steps
- **Describe the current behavior** and **explain the behavior you expected**
- **Explain why this enhancement would be useful** to most MetaImGUI users

### Contributing Code

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run tests and ensure they pass
5. Commit your changes with clear commit messages
6. Push to your branch
7. Open a Pull Request

## Development Setup

### Prerequisites

- CMake 3.16 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)
- Git

### Setting Up Your Development Environment

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/MetaImGUI.git
cd MetaImGUI

# Add upstream remote
git remote add upstream https://github.com/andynicholson/MetaImGUI.git

# Setup dependencies
./setup_dependencies.sh  # Linux/macOS
# or
.\setup_dependencies.bat  # Windows

# Build the project
./build.sh  # Linux/macOS
# or
.\build.bat  # Windows

# Run tests
cd build
ctest --output-on-failure
```

### Keeping Your Fork Updated

```bash
git fetch upstream
git checkout main
git merge upstream/main
```

## Coding Standards

### C++ Style Guide

We follow a consistent coding style based on LLVM:

- **C++ Standard**: C++20
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 120 characters maximum
- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `WindowManager`)
  - Functions/Methods: `PascalCase` (e.g., `Initialize()`)
  - Variables: `camelCase` with `m_` prefix for members (e.g., `m_windowManager`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `DEFAULT_WIDTH`)
  - Namespaces: `PascalCase` (e.g., `MetaImGUI`)
- **Header Guards**: `#pragma once`
- **Braces**: K&R style (opening brace on same line)

### Code Formatting

We use `clang-format` for automatic code formatting. Before submitting:

```bash
# Format all source files
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Or enable the pre-commit hook
cp .pre-commit-hook.sh .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

### Documentation

- Add Doxygen-style comments for all public APIs
- Document complex algorithms and design decisions
- Update README.md and other documentation when adding features
- Include code examples where appropriate

Example:

```cpp
/**
 * @brief Initialize the application and all subsystems
 *
 * This method sets up the window manager, UI renderer, and other
 * core components. It must be called before Run().
 *
 * @return true if initialization succeeded, false otherwise
 */
bool Initialize();
```

### Testing

- Write tests for all new features
- Ensure existing tests pass: `ctest --test-dir build`
- Aim for >70% code coverage
- Use Catch2 BDD style for test organization

Example:

```cpp
TEST_CASE("ConfigManager saves settings", "[config]") {
    SECTION("Window position is persisted") {
        ConfigManager config;
        config.SetWindowPosition(100, 200);
        config.Save();

        ConfigManager loaded;
        loaded.Load();
        REQUIRE(loaded.GetWindowPosition() == std::make_pair(100, 200));
    }
}
```

## Pull Request Process

1. **Update Documentation**: Ensure README.md and other docs reflect your changes
2. **Add Tests**: Include tests for new functionality
3. **Update CHANGELOG.md**: Add entry under "Unreleased" section
4. **Run All Tests**: Verify `ctest` passes on your system
5. **Format Code**: Run `clang-format` on all modified files
6. **Write Clear Commit Messages**:
   - Use present tense ("Add feature" not "Added feature")
   - Use imperative mood ("Move cursor to..." not "Moves cursor to...")
   - Reference issues and pull requests
   - First line should be 50 characters or less
   - Provide detailed description after blank line if needed

Example commit message:
```
Add ConfigManager for settings persistence

Implements a JSON-based configuration system that saves:
- Window position and size
- Theme selection
- Recent files list

Resolves #123
```

7. **Create Pull Request**:
   - Fill out the PR template completely
   - Link related issues
   - Add screenshots/videos for UI changes
   - Request review from maintainers

8. **Respond to Review Feedback**: Address all comments promptly

9. **CI/CD**: Ensure all GitHub Actions checks pass

### PR Checklist

Before submitting, verify:

- [ ] Code follows the project's style guidelines
- [ ] Self-review of code completed
- [ ] Code comments added for complex logic
- [ ] Documentation updated (README, comments, etc.)
- [ ] Tests added/updated and passing
- [ ] CHANGELOG.md updated
- [ ] No compiler warnings introduced
- [ ] clang-format applied
- [ ] Commits are clean and logical

## Project Structure

Understanding the project layout helps when contributing:

```
MetaImGUI/
├── src/                    # Source files
│   ├── Application.cpp    # Main application logic
│   ├── WindowManager.cpp  # Window management
│   ├── UIRenderer.cpp     # UI rendering
│   └── ...
├── include/               # Header files
├── tests/                 # Unit tests
├── external/              # Third-party dependencies
├── cmake/                 # CMake modules
├── .github/workflows/     # CI/CD configuration
└── packaging/             # Distribution scripts
```

### Adding New Classes

1. Create header in `include/YourClass.h`
2. Create implementation in `src/YourClass.cpp`
3. Add to `CMakeLists.txt`
4. Add tests in `tests/test_your_class.cpp`
5. Update documentation

## Branch Naming

Use descriptive branch names:

- `feature/short-description` - New features
- `bugfix/issue-number-description` - Bug fixes
- `docs/what-changed` - Documentation updates
- `refactor/what-changed` - Code refactoring

## Commit Messages

Follow these guidelines:

- Use the imperative mood ("Add feature" not "Added feature")
- Capitalize the first letter
- No period at the end of the subject line
- Limit subject line to 50 characters
- Wrap body at 72 characters
- Explain what and why, not how

## Getting Help

- **Discord/Chat**: [Link to chat if available]
- **GitHub Discussions**: Use for questions and ideas
- **GitHub Issues**: Use for bugs and features
- **Email**: Contact maintainers at [email if available]

## Recognition

Contributors will be:
- Added to the AUTHORS file
- Credited in release notes
- Mentioned in the project README

## License

By contributing, you agree that your contributions will be licensed under the GPL v3 License.

## Questions?

Don't hesitate to ask questions! Open an issue or reach out to the maintainers. We're here to help!

---

Thank you for contributing to MetaImGUI! Your efforts help make this template better for everyone. 🎉

