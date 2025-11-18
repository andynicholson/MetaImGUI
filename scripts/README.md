# GitHub Actions Monitoring Scripts

This directory contains scripts to monitor and manage GitHub Actions workflows for the MetaImGUI project.

## 📋 Available Scripts

### 1. `monitor_actions.sh` - Full Monitoring Script (Linux/macOS)
**Features:**
- Real-time workflow status monitoring
- Download and extract workflow logs
- Watch mode with auto-refresh
- Detailed job-level information
- Colored output for easy reading

**Usage:**
```bash
# Show current status
./scripts/monitor_actions.sh

# Watch workflows in real-time (refreshes every 30 seconds)
./scripts/monitor_actions.sh watch

# Watch with custom interval (10 seconds)
./scripts/monitor_actions.sh watch 10

# Download logs for latest runs
./scripts/monitor_actions.sh logs

# Show help
./scripts/monitor_actions.sh help
```

**Requirements:**
- `jq` for JSON parsing: `sudo apt-get install jq`
- `curl` for API requests
- Optional: `gh` (GitHub CLI) for enhanced functionality

### 2. `monitor_actions.ps1` - PowerShell Version (Windows)
**Features:**
- Same functionality as the bash version
- Windows-native PowerShell implementation
- Colorized output using PowerShell colors

**Usage:**
```powershell
# Show current status
.\scripts\monitor_actions.ps1

# Watch workflows in real-time
.\scripts\monitor_actions.ps1 -Command watch

# Watch with custom interval
.\scripts\monitor_actions.ps1 -Command watch -Interval 10

# Download logs
.\scripts\monitor_actions.ps1 -Command logs

# Show help
.\scripts\monitor_actions.ps1 -Command help
```

### 3. `quick_status.sh` - Simple Status Check
**Features:**
- Provides direct URLs to GitHub Actions
- Shows summary of recent workflow status
- Lists common troubleshooting steps
- No API dependencies

**Usage:**
```bash
./scripts/quick_status.sh
```

## 🔧 Setup Instructions

### Linux/Ubuntu Setup
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y jq curl

# Make scripts executable
chmod +x scripts/monitor_actions.sh
chmod +x scripts/quick_status.sh

# Optional: Install GitHub CLI for enhanced functionality
# Follow instructions at: https://cli.github.com/
```

### Windows Setup
```powershell
# No additional setup required for basic functionality
# PowerShell and basic Windows tools are sufficient

# Optional: Install GitHub CLI for enhanced functionality
# Download from: https://cli.github.com/
```

## 🚀 Quick Start

### Check Current Status
```bash
# Linux/macOS
./scripts/quick_status.sh

# Windows
.\scripts\monitor_actions.ps1
```

### Monitor Builds in Real-time
```bash
# Linux/macOS
./scripts/monitor_actions.sh watch

# Windows
.\scripts\monitor_actions.ps1 -Command watch
```

### Download Build Logs
```bash
# Linux/macOS
./scripts/monitor_actions.sh logs

# Windows
.\scripts\monitor_actions.ps1 -Command logs
```

## 📊 Output Explanation

### Status Indicators
- ✅ **Green**: Success
- ❌ **Red**: Failure
- ⏳ **Yellow**: Pending/Cancelled
- 🔄 **Blue**: Running/In Progress

### Workflow Information
- **Run ID**: Unique identifier for the workflow run
- **Created**: When the workflow was triggered
- **Branch**: Git branch that triggered the workflow
- **Commit**: Short commit hash (first 7 characters)
- **URL**: Direct link to the workflow run on GitHub

### Job Details
Each workflow run contains multiple jobs:
- **CI Workflow**: `build (ubuntu-latest, Debug)`, `build (ubuntu-latest, Release)`, `build (windows-latest, Debug)`, `build (windows-latest, Release)`
- **Release Workflow**: `release (ubuntu-latest, linux, MetaImGUI)`, `release (windows-latest, windows, MetaImGUI.exe)`

## 🔍 Troubleshooting

### Common Issues

#### 1. API Authentication Errors
**Problem**: Getting "Not Found" or "401 Unauthorized" errors
**Solution**: 
- For public repos: No authentication needed
- For private repos: Install and configure GitHub CLI
```bash
gh auth login
```

#### 2. jq Command Not Found
**Problem**: `jq: command not found`
**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install jq

# macOS
brew install jq

# Windows (using Chocolatey)
choco install jq
```

#### 3. Scripts Not Executable
**Problem**: Permission denied when running scripts
**Solution**:
```bash
chmod +x scripts/*.sh
```

#### 4. PowerShell Execution Policy
**Problem**: Cannot run PowerShell scripts
**Solution**:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Advanced Usage

#### Authentication with GitHub CLI
For private repositories or rate limiting issues:
```bash
# Install GitHub CLI
# Ubuntu/Debian
sudo apt-get install gh

# Login and authenticate
gh auth login

# Test authentication
gh api user
```

#### Custom Configuration
Edit the configuration section in the scripts to match your repository:
```bash
# In monitor_actions.sh
REPO_OWNER="your-username"
REPO_NAME="your-repo-name"
```

## 📁 Log Files

Downloaded logs are stored in:
- `./workflow_logs/` directory
- Format: `{WorkflowName}_{RunID}.zip`
- Extracted to: `{WorkflowName}_{RunID}/` directory

## 🔗 Direct Links

When scripts can't access the API, use these direct links:

### GitHub Actions Pages
- **Main Actions**: https://github.com/your-username/MetaImGUI/actions
- **CI Workflow**: https://github.com/your-username/MetaImGUI/actions/workflows/ci.yml
- **Release Workflow**: https://github.com/your-username/MetaImGUI/actions/workflows/release.yml

### Repository Management
- **Settings**: https://github.com/your-username/MetaImGUI/settings/actions
- **Secrets**: https://github.com/your-username/MetaImGUI/settings/secrets/actions

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub CLI Documentation](https://cli.github.com/)
- [GitHub REST API Actions](https://docs.github.com/en/rest/actions)

## 🤝 Contributing

When adding new monitoring features:
1. Test on both Linux and Windows
2. Update this README with new functionality
3. Ensure backward compatibility
4. Add proper error handling 