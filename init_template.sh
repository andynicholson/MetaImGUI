#!/bin/bash

# MetaImGUI Template Initialization Script
# This script helps you set up a new project from the MetaImGUI template

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔═══════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         MetaImGUI Template Initialization Script         ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════════╝${NC}"
echo ""

# Function to print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Interactive mode by default
INTERACTIVE=true
PROJECT_NAME=""
NAMESPACE=""
AUTHOR=""
DESCRIPTION=""
GITHUB_USERNAME=""
KEEP_META=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --name)
            PROJECT_NAME="$2"
            shift 2
            ;;
        --namespace)
            NAMESPACE="$2"
            shift 2
            ;;
        --author)
            AUTHOR="$2"
            shift 2
            ;;
        --description)
            DESCRIPTION="$2"
            shift 2
            ;;
        --github-user)
            GITHUB_USERNAME="$2"
            shift 2
            ;;
        --no-interactive)
            INTERACTIVE=false
            shift
            ;;
        --keep-meta)
            KEEP_META="yes"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --name NAME              Project name (e.g., MyApp)"
            echo "  --namespace NAMESPACE    C++ namespace (e.g., MyApp)"
            echo "  --author AUTHOR          Author name"
            echo "  --description DESC       Project description"
            echo "  --github-user USERNAME   GitHub username"
            echo "  --no-interactive         Non-interactive mode (uses defaults/args)"
            echo "  --keep-meta              Keep meta-project documentation"
            echo "  --help, -h               Show this help message"
            echo ""
            echo "Example:"
            echo "  $0 --name MyAwesomeApp --namespace MyApp --author \"John Doe\""
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Interactive prompts if not provided
if [ "$INTERACTIVE" = true ]; then
    echo -e "${GREEN}Let's set up your new ImGui project!${NC}"
    echo ""

    if [ -z "$PROJECT_NAME" ]; then
        read -p "$(echo -e ${BLUE}Enter project name${NC} [MyImGuiApp]): " PROJECT_NAME
        PROJECT_NAME=${PROJECT_NAME:-MyImGuiApp}
    fi

    if [ -z "$NAMESPACE" ]; then
        read -p "$(echo -e ${BLUE}Enter C++ namespace${NC} [$PROJECT_NAME]): " NAMESPACE
        NAMESPACE=${NAMESPACE:-$PROJECT_NAME}
    fi

    if [ -z "$AUTHOR" ]; then
        # Try to get git user name
        GIT_USER=$(git config user.name 2>/dev/null || echo "")
        read -p "$(echo -e ${BLUE}Enter author name${NC} [$GIT_USER]): " AUTHOR
        AUTHOR=${AUTHOR:-$GIT_USER}
    fi

    if [ -z "$DESCRIPTION" ]; then
        read -p "$(echo -e ${BLUE}Enter project description${NC} [An ImGui-based application]): " DESCRIPTION
        DESCRIPTION=${DESCRIPTION:-"An ImGui-based application"}
    fi

    if [ -z "$GITHUB_USERNAME" ]; then
        # Try to extract from git remote
        GIT_REMOTE=$(git config --get remote.origin.url 2>/dev/null || echo "")
        if [[ $GIT_REMOTE =~ github.com[:/]([^/]+)/ ]]; then
            GITHUB_USERNAME="${BASH_REMATCH[1]}"
        fi
        read -p "$(echo -e ${BLUE}Enter GitHub username${NC} [$GITHUB_USERNAME]): " INPUT_GITHUB
        GITHUB_USERNAME=${INPUT_GITHUB:-$GITHUB_USERNAME}
    fi

    if [ -z "$KEEP_META" ]; then
        read -p "$(echo -e ${YELLOW}Keep meta-project documentation?${NC} [y/N]): " KEEP_META
    fi

    echo ""
    echo -e "${YELLOW}Configuration Summary:${NC}"
    echo "  Project Name:   $PROJECT_NAME"
    echo "  Namespace:      $NAMESPACE"
    echo "  Author:         $AUTHOR"
    echo "  Description:    $DESCRIPTION"
    echo "  GitHub User:    $GITHUB_USERNAME"
    echo "  Keep Meta Docs: ${KEEP_META:-no}"
    echo ""
    read -p "$(echo -e ${YELLOW}Proceed with initialization?${NC} [y/N]): " CONFIRM

    if [ "$CONFIRM" != "y" ] && [ "$CONFIRM" != "Y" ]; then
        print_info "Initialization cancelled."
        exit 0
    fi
fi

# Validate required fields
if [ -z "$PROJECT_NAME" ]; then
    print_error "Project name is required"
    exit 1
fi

if [ -z "$NAMESPACE" ]; then
    NAMESPACE=$PROJECT_NAME
fi

print_info "Starting project initialization..."
echo ""

# Backup original files
print_info "Creating backup of original files..."
mkdir -p .template_backup
cp CMakeLists.txt .template_backup/
cp src/main.cpp .template_backup/
cp src/Application.cpp .template_backup/
cp include/Application.h .template_backup/
cp README.md .template_backup/
print_success "Backup created in .template_backup/"

# Update CMakeLists.txt
print_info "Updating CMakeLists.txt..."
sed -i.bak "s/MetaImGUI/$PROJECT_NAME/g" CMakeLists.txt
rm CMakeLists.txt.bak 2>/dev/null || true
print_success "CMakeLists.txt updated"

# Update namespace in source files and tests
print_info "Updating namespace in source files..."
find src include tests -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i.bak "s/namespace MetaImGUI/namespace $NAMESPACE/g" {} \;
find src include tests -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i.bak "s/MetaImGUI::/${NAMESPACE}::/g" {} \;
find src include tests -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i.bak "s/using namespace MetaImGUI;/using namespace $NAMESPACE;/g" {} \;
# Also update the expected project name in tests
find tests -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i.bak "s/\"MetaImGUI\"/\"$PROJECT_NAME\"/g" {} \;
find . -name "*.bak" -delete
print_success "Namespace updated to $NAMESPACE"

# Update window title
print_info "Updating application strings..."
sed -i.bak "s/MetaImGUI - ImGui Application Template/$PROJECT_NAME/g" include/Application.h
sed -i.bak "s/About MetaImGUI/About $PROJECT_NAME/g" src/Application.cpp
sed -i.bak "s/MetaImGUI v/${PROJECT_NAME} v/g" src/Application.cpp
rm include/Application.h.bak src/Application.cpp.bak 2>/dev/null || true
print_success "Application strings updated"

# Update version.h.in
print_info "Updating version header template..."
sed -i.bak "s/namespace MetaImGUI/namespace $NAMESPACE/g" include/version.h.in
rm include/version.h.in.bak 2>/dev/null || true
print_success "Version header template updated"

# Update README.md
print_info "Creating new README.md..."
cat > README.md << EOF
# $PROJECT_NAME

$DESCRIPTION

## Overview

$PROJECT_NAME is built using the MetaImGUI template, providing a modern C++20 ImGui application with cross-platform support.

## Features

- **Modern C++20** codebase with proper error handling
- **Cross-platform** support (Linux, Windows, macOS)
- **ImGui v1.92.4** integration with GLFW and OpenGL 4.6
- **Automated CI/CD** with GitHub Actions
- **Version management** from git tags
- **Professional build system** using CMake

## Quick Start

### Prerequisites

- CMake 3.16 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)
- Git (for downloading dependencies)

### Setup

1. **Setup dependencies:**
   \`\`\`bash
   chmod +x setup_dependencies.sh
   ./setup_dependencies.sh
   \`\`\`

2. **Build the project:**
   \`\`\`bash
   chmod +x build.sh
   ./build.sh
   \`\`\`

3. **Run the application:**
   \`\`\`bash
   ./build/$PROJECT_NAME
   \`\`\`

## Building Manually

\`\`\`bash
mkdir build
cd build
cmake ..
make -j\$(nproc)
\`\`\`

## Development

See the [MetaImGUI documentation](https://github.com/your-original-repo/MetaImGUI) for information about:
- Adding new features
- Project structure
- Build system customization
- CI/CD workflows

## License

TODO: Add your license here

## Author

$AUTHOR

## Acknowledgments

- Built with [MetaImGUI](https://github.com/your-original-repo/MetaImGUI) template
- **ImGui** by Omar Cornut for the excellent GUI library
- **GLFW** team for cross-platform window management
EOF
print_success "README.md created"

# Update build scripts
print_info "Updating build scripts..."
sed -i.bak "s/MetaImGUI/$PROJECT_NAME/g" build.sh
sed -i.bak "s/MetaImGUI/$PROJECT_NAME/g" build.bat 2>/dev/null || true
rm build.sh.bak 2>/dev/null || true
rm build.bat.bak 2>/dev/null || true
print_success "Build scripts updated"

# Update setup scripts
print_info "Updating setup scripts..."
sed -i.bak "s/MetaImGUI/$PROJECT_NAME/g" setup_dependencies.sh
sed -i.bak "s/MetaImGUI/$PROJECT_NAME/g" setup_dependencies.bat 2>/dev/null || true
rm setup_dependencies.sh.bak 2>/dev/null || true
rm setup_dependencies.bat.bak 2>/dev/null || true
print_success "Setup scripts updated"

# Update monitoring scripts if GitHub username is provided
if [ -n "$GITHUB_USERNAME" ]; then
    print_info "Updating monitoring scripts with GitHub username..."
    if [ -f "scripts/monitor_actions.sh" ]; then
        sed -i.bak "s/REPO_OWNER=\".*\"/REPO_OWNER=\"$GITHUB_USERNAME\"/" scripts/monitor_actions.sh
        sed -i.bak "s/REPO_NAME=\".*\"/REPO_NAME=\"$PROJECT_NAME\"/" scripts/monitor_actions.sh
        rm scripts/monitor_actions.sh.bak 2>/dev/null || true
    fi
    if [ -f "scripts/monitor_actions.ps1" ]; then
        sed -i.bak "s/\\\$RepoOwner = \".*\"/\$RepoOwner = \"$GITHUB_USERNAME\"/" scripts/monitor_actions.ps1
        sed -i.bak "s/\\\$RepoName = \".*\"/\$RepoName = \"$PROJECT_NAME\"/" scripts/monitor_actions.ps1
        rm scripts/monitor_actions.ps1.bak 2>/dev/null || true
    fi
    print_success "Monitoring scripts updated"
fi

# Clean build directory
if [ -d "build" ]; then
    print_info "Cleaning old build directory..."
    rm -rf build/*
    print_success "Build directory cleaned"
fi

# Remove meta-project documentation if requested
if [ "$KEEP_META" != "y" ] && [ "$KEEP_META" != "Y" ]; then
    print_info "Removing meta-project documentation..."
    rm -f init_template.sh init_template.ps1 2>/dev/null || true
    print_success "Meta-project files removed"
fi

# Git operations
print_info "Updating git configuration..."
if [ -d ".git" ]; then
    # Create new initial commit
    read -p "$(echo -e ${YELLOW}Create a new initial commit?${NC} [y/N]): " NEW_COMMIT
    if [ "$NEW_COMMIT" = "y" ] || [ "$NEW_COMMIT" = "Y" ]; then
        git add -A
        git commit -m "Initialize $PROJECT_NAME from MetaImGUI template

Project: $PROJECT_NAME
Namespace: $NAMESPACE
Author: $AUTHOR
Description: $DESCRIPTION"
        print_success "New commit created"
    fi
fi

echo ""
echo -e "${GREEN}╔═══════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║              Initialization Complete! 🎉                  ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Next steps:${NC}"
echo "  1. Run: ./setup_dependencies.sh"
echo "  2. Run: ./build.sh"
echo "  3. Run: ./build/$PROJECT_NAME"
echo ""
echo -e "${BLUE}Don't forget to:${NC}"
echo "  - Update README.md with your project details"
echo "  - Add a LICENSE file"
echo "  - Set up your GitHub repository"
echo "  - Configure GitHub Actions secrets if needed"
echo "  - Create an initial git tag for versioning: git tag v1.0.0"
echo ""
print_success "Your project is ready to go!"

