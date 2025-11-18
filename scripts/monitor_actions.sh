#!/bin/bash

# GitHub Actions Monitor Script - MetaImGUI Edition
# Real-time monitoring with auto-detection and robust error handling

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
LOGS_DIR="./workflow_logs"
CONFIG_FILE="$HOME/.metaimgui_monitor_config"

# Create logs directory
mkdir -p "$LOGS_DIR"

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "success") echo -e "${GREEN}✅ $message${NC}" ;;
        "failure") echo -e "${RED}❌ $message${NC}" ;;
        "pending") echo -e "${YELLOW}⏳ $message${NC}" ;;
        "running") echo -e "${BLUE}🔄 $message${NC}" ;;
        "info") echo -e "${CYAN}ℹ️  $message${NC}" ;;
        "warning") echo -e "${YELLOW}⚠️  $message${NC}" ;;
        "header") echo -e "${PURPLE}📊 $message${NC}" ;;
        *) echo -e "$message" ;;
    esac
}

# Function to auto-detect repository details
detect_repository() {
    print_status "info" "Auto-detecting repository details..."

    # Check if we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        print_status "failure" "Not in a git repository. Please run this script from the MetaImGUI directory."
        exit 1
    fi

    # Try to get remote origin URL
    local remote_url=$(git config --get remote.origin.url 2>/dev/null || echo "")

    if [[ -z "$remote_url" ]]; then
        print_status "failure" "No remote origin found. Please set up git remote."
        exit 1
    fi

    # Parse GitHub repository from remote URL
    if [[ "$remote_url" =~ github\.com[:/]([^/]+)/([^/.]+) ]]; then
        REPO_OWNER="${BASH_REMATCH[1]}"
        REPO_NAME="${BASH_REMATCH[2]}"
        print_status "success" "Detected repository: $REPO_OWNER/$REPO_NAME"
    else
        print_status "failure" "Could not parse GitHub repository from remote URL: $remote_url"
        exit 1
    fi

    # Save to config for future runs
    echo "REPO_OWNER=$REPO_OWNER" > "$CONFIG_FILE"
    echo "REPO_NAME=$REPO_NAME" >> "$CONFIG_FILE"
}

# Function to load or detect repository
load_repository() {
    if [[ -f "$CONFIG_FILE" ]]; then
        source "$CONFIG_FILE"
        print_status "info" "Using repository: $REPO_OWNER/$REPO_NAME"
    else
        detect_repository
    fi
}

# Function to check and setup gh CLI
check_and_setup_gh() {
    local retry_count=0
    local max_retries=3

    while [[ $retry_count -lt $max_retries ]]; do
        # Check if gh CLI is installed
        if ! command -v gh &> /dev/null; then
            print_status "failure" "GitHub CLI (gh) is not installed."
            echo
            print_status "info" "To install GitHub CLI:"
            echo "  Ubuntu/Debian: sudo apt install gh"
            echo "  macOS: brew install gh"
            echo "  Or visit: https://cli.github.com/"
            echo
            read -p "Would you like to try without gh CLI using curl? (y/n): " use_curl
            if [[ "$use_curl" =~ ^[Yy] ]]; then
                return 1  # Use curl fallback
            else
                exit 1
            fi
        fi

        # Check if authenticated
        if gh auth status &> /dev/null; then
            print_status "success" "GitHub CLI is authenticated and ready"
            return 0
        else
            print_status "warning" "GitHub CLI authentication failed (attempt $((retry_count + 1))/$max_retries)"
            echo
            print_status "info" "GitHub CLI needs authentication to access repository data."
            echo
            echo "Available authentication methods:"
            echo "1. Browser authentication (recommended)"
            echo "2. Personal access token"
            echo "3. Skip and use curl (limited functionality)"
            echo
            read -p "Choose option (1-3): " auth_choice

            case $auth_choice in
                1)
                    print_status "info" "Starting browser authentication..."
                    if gh auth login; then
                        print_status "success" "Authentication successful!"
                        return 0
                    else
                        print_status "failure" "Browser authentication failed"
                    fi
                    ;;
                2)
                    print_status "info" "You can create a personal access token at:"
                    echo "https://github.com/settings/tokens"
                    echo "Required scopes: repo, workflow"
                    read -p "Enter your personal access token: " -s token
                    echo
                    if echo "$token" | gh auth login --with-token; then
                        print_status "success" "Token authentication successful!"
                        return 0
                    else
                        print_status "failure" "Token authentication failed"
                    fi
                    ;;
                3)
                    print_status "warning" "Using curl fallback (some features may not work)"
                    return 1
                    ;;
                *)
                    print_status "failure" "Invalid choice"
                    ;;
            esac
        fi

        retry_count=$((retry_count + 1))
        if [[ $retry_count -lt $max_retries ]]; then
            read -p "Would you like to try again? (y/n): " try_again
            if [[ ! "$try_again" =~ ^[Yy] ]]; then
                break
            fi
        fi
    done

    print_status "failure" "Could not set up GitHub CLI authentication after $max_retries attempts"
    read -p "Continue with curl fallback? (y/n): " use_curl
    if [[ "$use_curl" =~ ^[Yy] ]]; then
        return 1
    else
        exit 1
    fi
}

# Function to discover available workflows
discover_workflows() {
    print_status "info" "Discovering available workflows..."

    local workflows=()

    if [[ $USE_GH_CLI -eq 0 ]]; then
        # Use gh CLI
        while IFS= read -r workflow; do
            workflows+=("$workflow")
        done < <(gh api repos/$REPO_OWNER/$REPO_NAME/actions/workflows --jq '.workflows[] | .path | sub(".github/workflows/"; "") | sub(".yml$"; "") | sub(".yaml$"; "")')
    else
        # Use curl fallback
        local workflow_data=$(curl -s "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/workflows")
        while IFS= read -r workflow; do
            workflows+=("$workflow")
        done < <(echo "$workflow_data" | jq -r '.workflows[]? | .path | sub(".github/workflows/"; "") | sub(".yml$"; "") | sub(".yaml$"; "")')
    fi

    if [[ ${#workflows[@]} -eq 0 ]]; then
        print_status "warning" "No workflows found in repository"
        return 1
    fi

    print_status "success" "Found ${#workflows[@]} workflow(s): ${workflows[*]}"
    WORKFLOWS=("${workflows[@]}")
    return 0
}

# Function to get workflow runs
get_workflow_runs() {
    local workflow_name=$1
    local max_runs=${2:-5}

    if [[ $USE_GH_CLI -eq 0 ]]; then
        # Use gh CLI
        gh api repos/$REPO_OWNER/$REPO_NAME/actions/workflows/$workflow_name/runs --jq ".workflow_runs[0:$max_runs] | .[] | {id: .id, name: .name, status: .status, conclusion: .conclusion, created_at: .created_at, head_branch: .head_branch, head_sha: .head_sha, html_url: .html_url}"
    else
        # Use curl fallback
        local workflow_id=$(curl -s "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/workflows" | jq -r ".workflows[] | select(.path == \".github/workflows/$workflow_name.yml\") | .id")
        if [[ -n "$workflow_id" ]]; then
            curl -s "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/workflows/$workflow_id/runs" | jq ".workflow_runs[0:$max_runs] | .[] | {id: .id, name: .name, status: .status, conclusion: .conclusion, created_at: .created_at, head_branch: .head_branch, head_sha: .head_sha, html_url: .html_url}"
        fi
    fi
}

# Function to display workflow status
display_workflow_status() {
    local workflow_name=$1
    local runs_data=$2

    print_status "header" "Workflow: $workflow_name"
    echo

    if [[ -z "$runs_data" ]]; then
        print_status "warning" "No runs found for $workflow_name"
        echo
        return
    fi

    while IFS= read -r run; do
        if [[ -n "$run" ]]; then
            local id=$(echo "$run" | jq -r '.id')
            local name=$(echo "$run" | jq -r '.name')
            local status=$(echo "$run" | jq -r '.status')
            local conclusion=$(echo "$run" | jq -r '.conclusion // "unknown"')
            local created_at=$(echo "$run" | jq -r '.created_at')
            local branch=$(echo "$run" | jq -r '.head_branch')
            local sha=$(echo "$run" | jq -r '.head_sha' | cut -c1-7)
            local url=$(echo "$run" | jq -r '.html_url')

            # Format date
            local date_str=$(date -d "$created_at" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created_at")

            # Determine status color
            local status_color=""
            local status_icon=""
            if [[ "$status" == "completed" ]]; then
                if [[ "$conclusion" == "success" ]]; then
                    status_color="$GREEN"
                    status_icon="✅"
                else
                    status_color="$RED"
                    status_icon="❌"
                fi
            elif [[ "$status" == "in_progress" ]]; then
                status_color="$BLUE"
                status_icon="🔄"
            else
                status_color="$YELLOW"
                status_icon="⏳"
            fi

            echo -e "${status_color}${status_icon} Run #$id${NC} - $name"
            echo -e "   Branch: $branch | Commit: $sha | Created: $date_str"
            echo -e "   Status: ${status_color}$status${NC}"
            if [[ "$status" == "completed" ]]; then
                echo -e "   Result: ${status_color}$conclusion${NC}"
            fi
            echo -e "   URL: $url"
            echo
        fi
    done < <(echo "$runs_data" | jq -c '.')
}

# Function to show all workflows status
show_all_workflows() {
    print_status "header" "GitHub Actions Status for $REPO_OWNER/$REPO_NAME"
    echo

    for workflow in "${WORKFLOWS[@]}"; do
        local runs_data=$(get_workflow_runs "$workflow.yml" 3)
        display_workflow_status "$workflow" "$runs_data"
    done
}

# Function to watch workflows
watch_workflows() {
    local interval=${1:-30}
    print_status "info" "Watching workflows (refresh every ${interval}s). Press Ctrl+C to stop."
    echo

    while true; do
        clear
        show_all_workflows
        print_status "info" "Last updated: $(date)"
        print_status "info" "Press Ctrl+C to stop watching"
        sleep "$interval"
    done
}

# Function to download logs
download_logs() {
    print_status "info" "Downloading logs for recent workflow runs..."

    for workflow in "${WORKFLOWS[@]}"; do
        local runs_data=$(get_workflow_runs "$workflow.yml" 2)

        while IFS= read -r run; do
            if [[ -n "$run" ]]; then
                local id=$(echo "$run" | jq -r '.id')
                local status=$(echo "$run" | jq -r '.status')

                if [[ "$status" == "completed" ]]; then
                    print_status "info" "Downloading logs for $workflow run #$id..."

                    local log_file="$LOGS_DIR/${workflow}_${id}.zip"
                    if [[ $USE_GH_CLI -eq 0 ]]; then
                        gh api repos/$REPO_OWNER/$REPO_NAME/actions/runs/$id/logs > "$log_file"
                    else
                        curl -s -H "Authorization: token $GITHUB_TOKEN" "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/runs/$id/logs" --output "$log_file"
                    fi

                    if [[ -f "$log_file" ]]; then
                        print_status "success" "Downloaded: $log_file"
                        # Extract logs
                        local extract_dir="$LOGS_DIR/${workflow}_${id}"
                        unzip -q "$log_file" -d "$extract_dir" 2>/dev/null || true
                        print_status "info" "Extracted to: $extract_dir"
                    else
                        print_status "failure" "Failed to download logs for run #$id"
                    fi
                fi
            fi
        done < <(echo "$runs_data" | jq -c '.')
    done
}

# Function to show help
show_help() {
    echo "MetaImGUI GitHub Actions Monitor"
    echo
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo
    echo "Commands:"
    echo "  (no args)    Show current workflow status"
    echo "  watch [sec]  Watch workflows in real-time (default: 30s interval)"
    echo "  logs         Download logs for recent workflow runs"
    echo "  help         Show this help message"
    echo
    echo "Examples:"
    echo "  $0                    # Show current status"
    echo "  $0 watch              # Watch with 30s refresh"
    echo "  $0 watch 10           # Watch with 10s refresh"
    echo "  $0 logs               # Download recent logs"
    echo
    echo "Requirements:"
    echo "  - jq (for JSON parsing)"
    echo "  - curl (for API requests)"
    echo "  - gh (GitHub CLI, optional but recommended)"
    echo
    echo "Setup:"
    echo "  Ubuntu/Debian: sudo apt install jq gh"
    echo "  macOS: brew install jq gh"
}

# Main script logic
main() {
    local command=${1:-status}
    local interval=${2:-30}

    case $command in
        "help"|"-h"|"--help")
            show_help
            exit 0
            ;;
        "watch")
            load_repository
            check_and_setup_gh
            discover_workflows
            watch_workflows "$interval"
            ;;
        "logs")
            load_repository
            check_and_setup_gh
            discover_workflows
            download_logs
            ;;
        "status"|*)
            load_repository
            check_and_setup_gh
            discover_workflows
            show_all_workflows
            ;;
    esac
}

# Initialize USE_GH_CLI variable
USE_GH_CLI=0

# Run main function with all arguments
main "$@"
