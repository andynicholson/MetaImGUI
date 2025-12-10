# MetaImGUI Template Initialization Script (PowerShell)
# This script helps you set up a new project from the MetaImGUI template

param(
    [string]$Name,
    [string]$Namespace,
    [string]$Author,
    [string]$Description,
    [string]$GitHubUser,
    [switch]$NoInteractive,
    [switch]$KeepMeta,
    [switch]$Help
)

# Colors for output
function Write-Info { Write-Host "[INFO] " -ForegroundColor Blue -NoNewline; Write-Host $args }
function Write-Success { Write-Host "[SUCCESS] " -ForegroundColor Green -NoNewline; Write-Host $args }
function Write-Warning { Write-Host "[WARNING] " -ForegroundColor Yellow -NoNewline; Write-Host $args }
function Write-Error { Write-Host "[ERROR] " -ForegroundColor Red -NoNewline; Write-Host $args }

# Display header
Write-Host ""
Write-Host "╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Blue
Write-Host "║         MetaImGUI Template Initialization Script         ║" -ForegroundColor Blue
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Blue
Write-Host ""

# Show help
if ($Help) {
    Write-Host "Usage: .\init_template.ps1 [OPTIONS]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Name NAME              Project name (e.g., MyApp)"
    Write-Host "  -Namespace NAMESPACE    C++ namespace (e.g., MyApp)"
    Write-Host "  -Author AUTHOR          Author name"
    Write-Host "  -Description DESC       Project description"
    Write-Host "  -GitHubUser USERNAME    GitHub username"
    Write-Host "  -NoInteractive          Non-interactive mode (uses defaults/args)"
    Write-Host "  -KeepMeta               Keep meta-project documentation"
    Write-Host "  -Help                   Show this help message"
    Write-Host ""
    Write-Host "Example:"
    Write-Host "  .\init_template.ps1 -Name MyAwesomeApp -Namespace MyApp -Author 'John Doe'"
    exit 0
}

# Check if we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
}

# Interactive prompts if not provided
if (-not $NoInteractive) {
    Write-Host "Let's set up your new ImGui project!" -ForegroundColor Green
    Write-Host ""

    if (-not $Name) {
        $Name = Read-Host "Enter project name [MyImGuiApp]"
        if ([string]::IsNullOrWhiteSpace($Name)) { $Name = "MyImGuiApp" }
    }

    if (-not $Namespace) {
        $Namespace = Read-Host "Enter C++ namespace [$Name]"
        if ([string]::IsNullOrWhiteSpace($Namespace)) { $Namespace = $Name }
    }

    if (-not $Author) {
        $GitUser = git config user.name 2>$null
        $prompt = if ($GitUser) { "Enter author name [$GitUser]" } else { "Enter author name" }
        $Author = Read-Host $prompt
        if ([string]::IsNullOrWhiteSpace($Author)) { $Author = $GitUser }
    }

    if (-not $Description) {
        $Description = Read-Host "Enter project description [An ImGui-based application]"
        if ([string]::IsNullOrWhiteSpace($Description)) { $Description = "An ImGui-based application" }
    }

    if (-not $GitHubUser) {
        $GitRemote = git config --get remote.origin.url 2>$null
        if ($GitRemote -match "github.com[:/]([^/]+)/") {
            $GitHubUser = $matches[1]
        }
        $prompt = if ($GitHubUser) { "Enter GitHub username [$GitHubUser]" } else { "Enter GitHub username" }
        $InputGitHub = Read-Host $prompt
        if (-not [string]::IsNullOrWhiteSpace($InputGitHub)) { $GitHubUser = $InputGitHub }
    }

    if (-not $KeepMeta) {
        $KeepMetaInput = Read-Host "Keep meta-project documentation? [y/N]"
        $KeepMeta = $KeepMetaInput -eq "y" -or $KeepMetaInput -eq "Y"
    }

    Write-Host ""
    Write-Host "Configuration Summary:" -ForegroundColor Yellow
    Write-Host "  Project Name:   $Name"
    Write-Host "  Namespace:      $Namespace"
    Write-Host "  Author:         $Author"
    Write-Host "  Description:    $Description"
    Write-Host "  GitHub User:    $GitHubUser"
    Write-Host "  Keep Meta Docs: $KeepMeta"
    Write-Host ""
    $Confirm = Read-Host "Proceed with initialization? [y/N]"

    if ($Confirm -ne "y" -and $Confirm -ne "Y") {
        Write-Info "Initialization cancelled."
        exit 0
    }
}

# Validate required fields
if ([string]::IsNullOrWhiteSpace($Name)) {
    Write-Error "Project name is required"
    exit 1
}

if ([string]::IsNullOrWhiteSpace($Namespace)) {
    $Namespace = $Name
}

Write-Info "Starting project initialization..."
Write-Host ""

# Backup original files
Write-Info "Creating backup of original files..."
New-Item -ItemType Directory -Force -Path ".template_backup" | Out-Null
Copy-Item "CMakeLists.txt" ".template_backup/"
Copy-Item "src/main.cpp" ".template_backup/"
Copy-Item "src/Application.cpp" ".template_backup/"
Copy-Item "include/Application.h" ".template_backup/"
Copy-Item "README.md" ".template_backup/"
Write-Success "Backup created in .template_backup/"

# Update CMakeLists.txt
Write-Info "Updating CMakeLists.txt..."
(Get-Content "CMakeLists.txt") -replace "MetaImGUI", $Name | Set-Content "CMakeLists.txt"
Write-Success "CMakeLists.txt updated"

# Update namespace in source files and tests
Write-Info "Updating namespace in source files..."
Get-ChildItem -Path src, include, tests -Include *.cpp, *.h -Recurse | ForEach-Object {
    (Get-Content $_.FullName) -replace "namespace MetaImGUI", "namespace $Namespace" | Set-Content $_.FullName
    (Get-Content $_.FullName) -replace "MetaImGUI::", "${Namespace}::" | Set-Content $_.FullName
    (Get-Content $_.FullName) -replace "using namespace MetaImGUI;", "using namespace $Namespace;" | Set-Content $_.FullName
}
# Also update the expected project name in tests
Get-ChildItem -Path tests -Include *.cpp, *.h -Recurse | ForEach-Object {
    (Get-Content $_.FullName) -replace '"MetaImGUI"', "`"$Name`"" | Set-Content $_.FullName
}
Write-Success "Namespace updated to $Namespace"

# Update window title
Write-Info "Updating application strings..."
(Get-Content "include/Application.h") -replace "MetaImGUI - ImGui Application Template", $Name | Set-Content "include/Application.h"
(Get-Content "src/Application.cpp") -replace "About MetaImGUI", "About $Name" | Set-Content "src/Application.cpp"
(Get-Content "src/Application.cpp") -replace "MetaImGUI v", "$Name v" | Set-Content "src/Application.cpp"
Write-Success "Application strings updated"

# Update version.h.in
Write-Info "Updating version header template..."
(Get-Content "include/version.h.in") -replace "namespace MetaImGUI", "namespace $Namespace" | Set-Content "include/version.h.in"
Write-Success "Version header template updated"

# Update README.md
Write-Info "Creating new README.md..."
@"
# $Name

$Description

## Overview

$Name is built using the MetaImGUI template, providing a modern C++20 ImGui application with cross-platform support.

## Features

- **Modern C++20** codebase with proper error handling
- **Cross-platform** support (Linux, Windows, macOS)
- **ImGui v1.90.4** integration with GLFW and OpenGL 3.3
- **Automated CI/CD** with GitHub Actions
- **Version management** from git tags
- **Professional build system** using CMake

## Quick Start

### Prerequisites

- CMake 3.16 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)
- Git (for downloading dependencies)

### Setup (Windows)

1. **Setup dependencies:**
   ``````powershell
   .\setup_dependencies.bat
   ``````

2. **Build the project:**
   ``````powershell
   .\build.bat
   ``````

3. **Run the application:**
   ``````powershell
   .\build\Release\$Name.exe
   ``````

### Setup (Linux/macOS)

1. **Setup dependencies:**
   ``````bash
   chmod +x setup_dependencies.sh
   ./setup_dependencies.sh
   ``````

2. **Build the project:**
   ``````bash
   chmod +x build.sh
   ./build.sh
   ``````

3. **Run the application:**
   ``````bash
   ./build/$Name
   ``````

## Development

See the [MetaImGUI documentation](https://github.com/your-original-repo/MetaImGUI) for information about:
- Adding new features
- Project structure
- Build system customization
- CI/CD workflows

## License

TODO: Add your license here

## Author

$Author

## Acknowledgments

- Built with [MetaImGUI](https://github.com/your-original-repo/MetaImGUI) template
- **ImGui** by Omar Cornut for the excellent GUI library
- **GLFW** team for cross-platform window management
"@ | Set-Content "README.md"
Write-Success "README.md created"

# Update build scripts
Write-Info "Updating build scripts..."
if (Test-Path "build.sh") {
    (Get-Content "build.sh") -replace "MetaImGUI", $Name | Set-Content "build.sh"
}
if (Test-Path "build.bat") {
    (Get-Content "build.bat") -replace "MetaImGUI", $Name | Set-Content "build.bat"
}
Write-Success "Build scripts updated"

# Update setup scripts
Write-Info "Updating setup scripts..."
if (Test-Path "setup_dependencies.sh") {
    (Get-Content "setup_dependencies.sh") -replace "MetaImGUI", $Name | Set-Content "setup_dependencies.sh"
}
if (Test-Path "setup_dependencies.bat") {
    (Get-Content "setup_dependencies.bat") -replace "MetaImGUI", $Name | Set-Content "setup_dependencies.bat"
}
Write-Success "Setup scripts updated"

# Update monitoring scripts if GitHub username is provided
if (-not [string]::IsNullOrWhiteSpace($GitHubUser)) {
    Write-Info "Updating monitoring scripts with GitHub username..."
    if (Test-Path "scripts/monitor_actions.sh") {
        (Get-Content "scripts/monitor_actions.sh") -replace 'REPO_OWNER=".*"', "REPO_OWNER=`"$GitHubUser`"" | Set-Content "scripts/monitor_actions.sh"
        (Get-Content "scripts/monitor_actions.sh") -replace 'REPO_NAME=".*"', "REPO_NAME=`"$Name`"" | Set-Content "scripts/monitor_actions.sh"
    }
    if (Test-Path "scripts/monitor_actions.ps1") {
        (Get-Content "scripts/monitor_actions.ps1") -replace '\$RepoOwner = ".*"', "`$RepoOwner = `"$GitHubUser`"" | Set-Content "scripts/monitor_actions.ps1"
        (Get-Content "scripts/monitor_actions.ps1") -replace '\$RepoName = ".*"', "`$RepoName = `"$Name`"" | Set-Content "scripts/monitor_actions.ps1"
    }
    Write-Success "Monitoring scripts updated"
}

# Clean build directory
if (Test-Path "build") {
    Write-Info "Cleaning old build directory..."
    Remove-Item "build\*" -Recurse -Force -ErrorAction SilentlyContinue
    Write-Success "Build directory cleaned"
}

# Remove meta-project documentation if requested
if (-not $KeepMeta) {
    Write-Info "Removing meta-project documentation..."
    Remove-Item "init_template.sh", "init_template.ps1" -ErrorAction SilentlyContinue
    Write-Success "Meta-project files removed"
}

# Git operations
Write-Info "Updating git configuration..."
if (Test-Path ".git") {
    $NewCommit = Read-Host "Create a new initial commit? [y/N]"
    if ($NewCommit -eq "y" -or $NewCommit -eq "Y") {
        git add -A
        git commit -m "Initialize $Name from MetaImGUI template

Project: $Name
Namespace: $Namespace
Author: $Author
Description: $Description"
        Write-Success "New commit created"
    }
}

Write-Host ""
Write-Host "╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║              Initialization Complete! 🎉                  ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Blue
Write-Host "  1. Run: .\setup_dependencies.bat (or .sh on Linux)"
Write-Host "  2. Run: .\build.bat (or .sh on Linux)"
Write-Host "  3. Run: .\build\Release\$Name.exe"
Write-Host ""
Write-Host "Don't forget to:" -ForegroundColor Blue
Write-Host "  - Update README.md with your project details"
Write-Host "  - Add a LICENSE file"
Write-Host "  - Set up your GitHub repository"
Write-Host "  - Configure GitHub Actions secrets if needed"
Write-Host "  - Create an initial git tag for versioning: git tag v1.0.0"
Write-Host ""
Write-Success "Your project is ready to go!"

