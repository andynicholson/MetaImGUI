# PowerShell script to create Windows installer
# Requires NSIS to be installed

param(
    [string]$Version = "1.0.0"
)

# Colors
function Write-Info { Write-Host "[INFO] " -ForegroundColor Blue -NoNewline; Write-Host $args }
function Write-Success { Write-Host "[SUCCESS] " -ForegroundColor Green -NoNewline; Write-Host $args }
function Write-Error { Write-Host "[ERROR] " -ForegroundColor Red -NoNewline; Write-Host $args }

Write-Host ""
Write-Host "Creating Windows Installer for MetaImGUI v$Version" -ForegroundColor Cyan
Write-Host ""

# Check if NSIS is installed
$nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"
if (-not (Test-Path $nsisPath)) {
    $nsisPath = "C:\Program Files\NSIS\makensis.exe"
    if (-not (Test-Path $nsisPath)) {
        Write-Error "NSIS not found. Please install NSIS from https://nsis.sourceforge.io/"
        Write-Host "After installing NSIS, run this script again."
        exit 1
    }
}

Write-Info "Found NSIS at: $nsisPath"

# Ensure we're in the project root
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Error "Please run this script from the project root directory"
    exit 1
}

# Create output directory
$outputDir = "packaging\output"
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

# Build the project if not already built
if (-not (Test-Path "build\Release\MetaImGUI.exe")) {
    Write-Info "Building project..."

    # Determine vcpkg path
    $vcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { "C:/vcpkg" }
    $vcpkgToolchain = "$vcpkgRoot/scripts/buildsystems/vcpkg.cmake"

    if (-not (Test-Path $vcpkgToolchain)) {
        Write-Error "vcpkg toolchain not found at: $vcpkgToolchain"
        Write-Host "Please install vcpkg or set VCPKG_ROOT environment variable"
        exit 1
    }

    Write-Info "Using vcpkg toolchain: $vcpkgToolchain"

    # Configure
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$vcpkgToolchain"

    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed"
        exit 1
    }

    # Build
    cmake --build build --config Release --parallel

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }
}

Write-Success "Build complete"

# Update version in NSI script
$nsiScript = "packaging\windows_installer.nsi"
if (Test-Path $nsiScript) {
    $content = Get-Content $nsiScript
    $content = $content -replace '!define PRODUCT_VERSION ".*"', "!define PRODUCT_VERSION `"$Version`""
    Set-Content $nsiScript $content
    Write-Info "Updated version to $Version in installer script"
}

# Build installer with NSIS
Write-Info "Building installer with NSIS..."
& $nsisPath $nsiScript

if ($LASTEXITCODE -eq 0) {
    Write-Success "Installer created successfully!"
    Write-Host ""
    Write-Host "Output:" -ForegroundColor Cyan
    Get-ChildItem $outputDir -Filter "*.exe" | ForEach-Object {
        Write-Host "  $($_.FullName)" -ForegroundColor Green
        Write-Host "  Size: $([math]::Round($_.Length / 1MB, 2)) MB"
    }
    Write-Host ""
} else {
    Write-Error "NSIS compilation failed"
    exit 1
}

# Create portable ZIP
Write-Info "Creating portable ZIP package..."
$portableDir = "MetaImGUI-$Version-portable"
$portablePath = "packaging\$portableDir"

# Clean and create directory
if (Test-Path $portablePath) {
    Remove-Item $portablePath -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $portablePath | Out-Null

# Copy executable
Copy-Item "build\Release\MetaImGUI.exe" $portablePath

# Determine vcpkg installed path
$vcpkgInstalled = if ($env:VCPKG_ROOT) { "$env:VCPKG_ROOT/installed/x64-windows/bin" } else { "C:\vcpkg\installed\x64-windows\bin" }

# Copy required DLLs from vcpkg
$requiredDlls = @(
    "glfw3.dll",
    "libcurl.dll",
    "zlib1.dll",
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll"
)

$optionalDlls = @(
    "msvcp140.dll",
    "vcruntime140.dll",
    "vcruntime140_1.dll",
    "msvcp140_atomic_wait.dll"
)

Write-Info "Copying required DLLs from vcpkg..."
foreach ($dll in $requiredDlls) {
    $dllPath = "$vcpkgInstalled\$dll"
    if (Test-Path $dllPath) {
        Copy-Item $dllPath $portablePath
        Write-Info "  ✓ $dll"
    } else {
        Write-Warning "  ✗ $dll not found at: $dllPath"
    }
}

Write-Info "Copying optional runtime DLLs..."
foreach ($dll in $optionalDlls) {
    $dllPath = "$vcpkgInstalled\$dll"
    if (Test-Path $dllPath) {
        Copy-Item $dllPath $portablePath
        Write-Info "  ✓ $dll"
    } else {
        Write-Info "  - $dll (not found, may be in system)"
    }
}

# Copy resources if they exist
if (Test-Path "resources") {
    Copy-Item "resources" $portablePath -Recurse
}

# Copy README
if (Test-Path "README.md") {
    Copy-Item "README.md" $portablePath
}

# Create ZIP
$zipFile = "$outputDir\MetaImGUI-$Version-windows-x64-portable.zip"
if (Test-Path $zipFile) {
    Remove-Item $zipFile -Force
}

Compress-Archive -Path $portablePath -DestinationPath $zipFile

Write-Success "Portable ZIP created: $zipFile"

# Cleanup
Remove-Item $portablePath -Recurse -Force

Write-Host ""
Write-Success "Windows packaging complete!"
Write-Host ""
Write-Host "Created packages:" -ForegroundColor Cyan
Get-ChildItem $outputDir | ForEach-Object {
    Write-Host "  $($_.Name)" -ForegroundColor Green
}
Write-Host ""

