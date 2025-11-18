# GitHub Actions Monitor Script - MetaImGUI Edition (PowerShell)
# Real-time monitoring with auto-detection and robust error handling

param(
    [Parameter(Position=0)]
    [string]$Command = "status",
    
    [Parameter(Position=1)]
    [int]$Interval = 30
)

# Colors for output
$Colors = @{
    Red = "Red"
    Green = "Green"
    Yellow = "Yellow"
    Blue = "Blue"
    Magenta = "Magenta"
    Cyan = "Cyan"
    White = "White"
}

# Configuration
$LogsDir = "./workflow_logs"
$ConfigFile = "$env:USERPROFILE\.metaimgui_monitor_config"

# Create logs directory
if (!(Test-Path $LogsDir)) {
    New-Item -ItemType Directory -Force -Path $LogsDir | Out-Null
}

# Function to print colored output
function Write-Status {
    param(
        [string]$Status,
        [string]$Message
    )
    
    $color = switch ($Status) {
        "success" { $Colors.Green }
        "failure" { $Colors.Red }
        "pending" { $Colors.Yellow }
        "running" { $Colors.Blue }
        "info" { $Colors.Cyan }
        "warning" { $Colors.Yellow }
        "header" { $Colors.Magenta }
        default { $Colors.White }
    }
    
    $icon = switch ($Status) {
        "success" { "✅" }
        "failure" { "❌" }
        "pending" { "⏳" }
        "running" { "🔄" }
        "info" { "ℹ️" }
        "warning" { "⚠️" }
        "header" { "📊" }
        default { "" }
    }
    
    Write-Host "$icon $Message" -ForegroundColor $color
}

# Function to auto-detect repository details
function Detect-Repository {
    Write-Status "info" "Auto-detecting repository details..."
    
    # Check if we're in a git repository
    try {
        $null = git rev-parse --git-dir 2>$null
    }
    catch {
        Write-Status "failure" "Not in a git repository. Please run this script from the MetaImGUI directory."
        exit 1
    }
    
    # Try to get remote origin URL
    $remoteUrl = git config --get remote.origin.url 2>$null
    
    if ([string]::IsNullOrEmpty($remoteUrl)) {
        Write-Status "failure" "No remote origin found. Please set up git remote."
        exit 1
    }
    
    # Parse GitHub repository from remote URL
    if ($remoteUrl -match "github\.com[:/]([^/]+)/([^/.]+)") {
        $script:RepoOwner = $matches[1]
        $script:RepoName = $matches[2]
        Write-Status "success" "Detected repository: $RepoOwner/$RepoName"
    }
    else {
        Write-Status "failure" "Could not parse GitHub repository from remote URL: $remoteUrl"
        exit 1
    }
    
    # Save to config for future runs
    "REPO_OWNER=$RepoOwner" | Out-File -FilePath $ConfigFile -Encoding UTF8
    "REPO_NAME=$RepoName" | Add-Content -Path $ConfigFile -Encoding UTF8
}

# Function to load or detect repository
function Load-Repository {
    if (Test-Path $ConfigFile) {
        Get-Content $ConfigFile | ForEach-Object {
            if ($_ -match "REPO_OWNER=(.+)") {
                $script:RepoOwner = $matches[1]
            }
            elseif ($_ -match "REPO_NAME=(.+)") {
                $script:RepoName = $matches[1]
            }
        }
        Write-Status "info" "Using repository: $RepoOwner/$RepoName"
    }
    else {
        Detect-Repository
    }
}

# Function to check and setup gh CLI
function Test-AndSetup-GitHubCLI {
    $retryCount = 0
    $maxRetries = 3
    
    while ($retryCount -lt $maxRetries) {
        # Check if gh CLI is installed
        if (!(Get-Command gh -ErrorAction SilentlyContinue)) {
            Write-Status "failure" "GitHub CLI (gh) is not installed."
            Write-Host ""
            Write-Status "info" "To install GitHub CLI:"
            Write-Host "  Download from: https://cli.github.com/"
            Write-Host "  Or use winget: winget install GitHub.cli"
            Write-Host ""
            $useCurl = Read-Host "Would you like to try without gh CLI using curl? (y/n)"
            if ($useCurl -match "^[Yy]") {
                return $false  # Use curl fallback
            }
            else {
                exit 1
            }
        }
        
        # Check if authenticated
        try {
            $null = gh auth status 2>$null
            Write-Status "success" "GitHub CLI is authenticated and ready"
            return $true
        }
        catch {
            Write-Status "warning" "GitHub CLI authentication failed (attempt $($retryCount + 1)/$maxRetries)"
            Write-Host ""
            Write-Status "info" "GitHub CLI needs authentication to access repository data."
            Write-Host ""
            Write-Host "Available authentication methods:"
            Write-Host "1. Browser authentication (recommended)"
            Write-Host "2. Personal access token"
            Write-Host "3. Skip and use curl (limited functionality)"
            Write-Host ""
            $authChoice = Read-Host "Choose option (1-3)"
            
            switch ($authChoice) {
                "1" {
                    Write-Status "info" "Starting browser authentication..."
                    try {
                        gh auth login
                        Write-Status "success" "Authentication successful!"
                        return $true
                    }
                    catch {
                        Write-Status "failure" "Browser authentication failed"
                    }
                }
                "2" {
                    Write-Status "info" "You can create a personal access token at:"
                    Write-Host "https://github.com/settings/tokens"
                    Write-Host "Required scopes: repo, workflow"
                    $token = Read-Host "Enter your personal access token" -AsSecureString
                    $plainToken = [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($token))
                    try {
                        $plainToken | gh auth login --with-token
                        Write-Status "success" "Token authentication successful!"
                        return $true
                    }
                    catch {
                        Write-Status "failure" "Token authentication failed"
                    }
                }
                "3" {
                    Write-Status "warning" "Using curl fallback (some features may not work)"
                    return $false
                }
                default {
                    Write-Status "failure" "Invalid choice"
                }
            }
        }
        
        $retryCount++
        if ($retryCount -lt $maxRetries) {
            $tryAgain = Read-Host "Would you like to try again? (y/n)"
            if ($tryAgain -notmatch "^[Yy]") {
                break
            }
        }
    }
    
    Write-Status "failure" "Could not set up GitHub CLI authentication after $maxRetries attempts"
    $useCurl = Read-Host "Continue with curl fallback? (y/n)"
    if ($useCurl -match "^[Yy]") {
        return $false
    }
    else {
        exit 1
    }
}

# Function to discover available workflows
function Discover-Workflows {
    Write-Status "info" "Discovering available workflows..."
    
    $workflows = @()
    
    if ($UseGitHubCLI) {
        # Use gh CLI
        try {
            $workflowData = gh api repos/$RepoOwner/$RepoName/actions/workflows --jq '.workflows[] | .path | sub(".github/workflows/"; "") | sub(".yml$"; "") | sub(".yaml$"; "")'
            $workflows = $workflowData -split "`n" | Where-Object { $_ -ne "" }
        }
        catch {
            Write-Status "warning" "Failed to get workflows via GitHub CLI"
        }
    }
    else {
        # Use curl fallback
        try {
            $workflowData = Invoke-RestMethod -Uri "https://api.github.com/repos/$RepoOwner/$RepoName/actions/workflows"
            $workflows = $workflowData.workflows | ForEach-Object { 
                $_.path -replace ".github/workflows/", "" -replace "\.(yml|yaml)$", ""
            }
        }
        catch {
            Write-Status "warning" "Failed to get workflows via curl"
        }
    }
    
    if ($workflows.Count -eq 0) {
        Write-Status "warning" "No workflows found in repository"
        return $false
    }
    
    Write-Status "success" "Found $($workflows.Count) workflow(s): $($workflows -join ', ')"
    $script:Workflows = $workflows
    return $true
}

# Function to get workflow runs
function Get-WorkflowRuns {
    param(
        [string]$WorkflowName,
        [int]$MaxRuns = 5
    )
    
    if ($UseGitHubCLI) {
        # Use gh CLI
        try {
            $runsData = gh api repos/$RepoOwner/$RepoName/actions/workflows/$WorkflowName/runs --jq ".workflow_runs[0:$MaxRuns] | .[] | {id: .id, name: .name, status: .status, conclusion: .conclusion, created_at: .created_at, head_branch: .head_branch, head_sha: .head_sha, html_url: .html_url}"
            return $runsData | ConvertFrom-Json
        }
        catch {
            Write-Status "warning" "Failed to get workflow runs for $WorkflowName"
            return @()
        }
    }
    else {
        # Use curl fallback
        try {
            $workflowData = Invoke-RestMethod -Uri "https://api.github.com/repos/$RepoOwner/$RepoName/actions/workflows"
            $workflowId = ($workflowData.workflows | Where-Object { $_.path -eq ".github/workflows/$WorkflowName.yml" }).id
            if ($workflowId) {
                $runsData = Invoke-RestMethod -Uri "https://api.github.com/repos/$RepoOwner/$RepoName/actions/workflows/$workflowId/runs"
                return $runsData.workflow_runs[0..([Math]::Min($MaxRuns-1, $runsData.workflow_runs.Count-1))]
            }
        }
        catch {
            Write-Status "warning" "Failed to get workflow runs for $WorkflowName"
        }
        return @()
    }
}

# Function to display workflow status
function Show-WorkflowStatus {
    param(
        [string]$WorkflowName,
        $RunsData
    )
    
    Write-Status "header" "Workflow: $WorkflowName"
    Write-Host ""
    
    if ($RunsData.Count -eq 0) {
        Write-Status "warning" "No runs found for $WorkflowName"
        Write-Host ""
        return
    }
    
    foreach ($run in $RunsData) {
        $id = $run.id
        $name = $run.name
        $status = $run.status
        $conclusion = if ($run.conclusion) { $run.conclusion } else { "unknown" }
        $createdAt = $run.created_at
        $branch = $run.head_branch
        $sha = $run.head_sha.Substring(0, 7)
        $url = $run.html_url
        
        # Format date
        $dateStr = [DateTime]::Parse($createdAt).ToString("yyyy-MM-dd HH:mm")
        
        # Determine status color
        $statusColor = $Colors.White
        $statusIcon = ""
        if ($status -eq "completed") {
            if ($conclusion -eq "success") {
                $statusColor = $Colors.Green
                $statusIcon = "✅"
            }
            else {
                $statusColor = $Colors.Red
                $statusIcon = "❌"
            }
        }
        elseif ($status -eq "in_progress") {
            $statusColor = $Colors.Blue
            $statusIcon = "🔄"
        }
        else {
            $statusColor = $Colors.Yellow
            $statusIcon = "⏳"
        }
        
        Write-Host "$statusIcon Run #$id - $name" -ForegroundColor $statusColor
        Write-Host "   Branch: $branch | Commit: $sha | Created: $dateStr"
        Write-Host "   Status: $status" -ForegroundColor $statusColor
        if ($status -eq "completed") {
            Write-Host "   Result: $conclusion" -ForegroundColor $statusColor
        }
        Write-Host "   URL: $url"
        Write-Host ""
    }
}

# Function to show all workflows status
function Show-AllWorkflows {
    Write-Status "header" "GitHub Actions Status for $RepoOwner/$RepoName"
    Write-Host ""
    
    foreach ($workflow in $Workflows) {
        $runsData = Get-WorkflowRuns "$workflow.yml" 3
        Show-WorkflowStatus $workflow $runsData
    }
}

# Function to watch workflows
function Watch-Workflows {
    param([int]$Interval = 30)
    
    Write-Status "info" "Watching workflows (refresh every ${Interval}s). Press Ctrl+C to stop."
    Write-Host ""
    
    while ($true) {
        Clear-Host
        Show-AllWorkflows
        Write-Status "info" "Last updated: $(Get-Date)"
        Write-Status "info" "Press Ctrl+C to stop watching"
        Start-Sleep -Seconds $Interval
    }
}

# Function to download logs
function Download-Logs {
    Write-Status "info" "Downloading logs for recent workflow runs..."
    
    foreach ($workflow in $Workflows) {
        $runsData = Get-WorkflowRuns "$workflow.yml" 2
        
        foreach ($run in $runsData) {
            $id = $run.id
            $status = $run.status
            
            if ($status -eq "completed") {
                Write-Status "info" "Downloading logs for $workflow run #$id..."
                
                $logFile = "$LogsDir/${workflow}_${id}.zip"
                try {
                    if ($UseGitHubCLI) {
                        gh api repos/$RepoOwner/$RepoName/actions/runs/$id/logs --output $logFile
                    }
                    else {
                        Invoke-WebRequest -Uri "https://api.github.com/repos/$RepoOwner/$RepoName/actions/runs/$id/logs" -OutFile $logFile
                    }
                    
                    if (Test-Path $logFile) {
                        Write-Status "success" "Downloaded: $logFile"
                        # Extract logs
                        $extractDir = "$LogsDir/${workflow}_${id}"
                        Expand-Archive -Path $logFile -DestinationPath $extractDir -Force
                        Write-Status "info" "Extracted to: $extractDir"
                    }
                    else {
                        Write-Status "failure" "Failed to download logs for run #$id"
                    }
                }
                catch {
                    Write-Status "failure" "Failed to download logs for run #$id"
                }
            }
        }
    }
}

# Function to show help
function Show-Help {
    Write-Host "MetaImGUI GitHub Actions Monitor (PowerShell)"
    Write-Host ""
    Write-Host "Usage: .\monitor_actions.ps1 [COMMAND] [OPTIONS]"
    Write-Host ""
    Write-Host "Commands:"
    Write-Host "  status        Show current workflow status (default)"
    Write-Host "  watch [sec]   Watch workflows in real-time (default: 30s interval)"
    Write-Host "  logs          Download logs for recent workflow runs"
    Write-Host "  help          Show this help message"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\monitor_actions.ps1              # Show current status"
    Write-Host "  .\monitor_actions.ps1 watch        # Watch with 30s refresh"
    Write-Host "  .\monitor_actions.ps1 watch 10     # Watch with 10s refresh"
    Write-Host "  .\monitor_actions.ps1 logs         # Download recent logs"
    Write-Host ""
    Write-Host "Requirements:"
    Write-Host "  - PowerShell 5.1 or higher"
    Write-Host "  - gh (GitHub CLI, optional but recommended)"
    Write-Host ""
    Write-Host "Setup:"
    Write-Host "  Download GitHub CLI from: https://cli.github.com/"
}

# Main script logic
function Main {
    param(
        [string]$Command = "status",
        [int]$Interval = 30
    )
    
    switch ($Command.ToLower()) {
        "help" {
            Show-Help
            exit 0
        }
        "watch" {
            Load-Repository
            $script:UseGitHubCLI = Test-AndSetup-GitHubCLI
            Discover-Workflows
            Watch-Workflows $Interval
        }
        "logs" {
            Load-Repository
            $script:UseGitHubCLI = Test-AndSetup-GitHubCLI
            Discover-Workflows
            Download-Logs
        }
        "status" {
            Load-Repository
            $script:UseGitHubCLI = Test-AndSetup-GitHubCLI
            Discover-Workflows
            Show-AllWorkflows
        }
        default {
            Load-Repository
            $script:UseGitHubCLI = Test-AndSetup-GitHubCLI
            Discover-Workflows
            Show-AllWorkflows
        }
    }
}

# Initialize variables
$script:UseGitHubCLI = $false
$script:RepoOwner = ""
$script:RepoName = ""
$script:Workflows = @()

# Run main function
Main -Command $Command -Interval $Interval 