#!/bin/bash

# GitHub Actions Monitor Script - MetaImGUI Edition
# Enhanced with comprehensive CI/CD monitoring, coverage, security, and error analysis

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Configuration
LOGS_DIR="./workflow_logs"
CONFIG_FILE="$HOME/.metaimgui_monitor_config"
CACHE_DIR="$HOME/.cache/metaimgui_monitor"

# Create necessary directories
mkdir -p "$LOGS_DIR"
mkdir -p "$CACHE_DIR"

# Workflow categories
declare -A WORKFLOW_CATEGORIES=(
    ["ci"]="ci.yml"
    ["release"]="release.yml"
    ["coverage"]="coverage.yml"
    ["codeql"]="codeql.yml"
    ["dependency-review"]="dependency-review.yml"
    ["static-analysis"]="static-analysis.yml"
    ["sanitizers"]="sanitizers.yml"
    ["benchmarks"]="benchmarks.yml"
    ["docs"]="docs.yml"
)

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
        "header") echo -e "${PURPLE}${BOLD}📊 $message${NC}" ;;
        "security") echo -e "${RED}🔒 $message${NC}" ;;
        "coverage") echo -e "${BLUE}📈 $message${NC}" ;;
        *) echo -e "$message" ;;
    esac
}

# Function to print section header
print_section() {
    local title=$1
    echo
    echo -e "${BOLD}${PURPLE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BOLD}${PURPLE}  $title${NC}"
    echo -e "${BOLD}${PURPLE}═══════════════════════════════════════════════════════════════${NC}"
    echo
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
    # Check if gh CLI is installed
    if ! command -v gh &> /dev/null; then
        print_status "failure" "GitHub CLI (gh) is not installed."
        echo
        print_status "info" "To install GitHub CLI:"
        echo "  Ubuntu/Debian: sudo apt install gh"
        echo "  macOS: brew install gh"
        echo "  Or visit: https://cli.github.com/"
        exit 1
    fi

    # Check if authenticated
    if ! gh auth status &> /dev/null; then
        print_status "warning" "GitHub CLI is not authenticated."
        print_status "info" "Please run: gh auth login"
        exit 1
    fi

    print_status "success" "GitHub CLI is authenticated and ready"
}

# Function to get all workflow runs with gh CLI
get_all_runs() {
    local limit=${1:-20}
    gh run list -R "$REPO_OWNER/$REPO_NAME" --limit "$limit" --json databaseId,displayTitle,headBranch,name,status,conclusion,createdAt,updatedAt,url,workflowName,event
}

# Function to get workflow run details
get_run_details() {
    local run_id=$1
    gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME" --json databaseId,displayTitle,headBranch,name,status,conclusion,createdAt,updatedAt,url,workflowName,event,jobs
}

# Function to get run logs
get_run_logs() {
    local run_id=$1
    gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME" --log
}

# Function to extract errors from logs
extract_errors_from_logs() {
    local run_id=$1
    local log_file="$LOGS_DIR/run_${run_id}_errors.txt"

    print_status "info" "Extracting errors from run #$run_id..."

    gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME" --log 2>/dev/null | \
        grep -iE "(error|fail|fatal|exception|assertion.*failed|segfault|core dumped)" | \
        head -n 50 > "$log_file"

    if [[ -s "$log_file" ]]; then
        echo -e "${RED}Found errors in run #$run_id:${NC}"
        cat "$log_file" | head -n 20
        if [[ $(wc -l < "$log_file") -gt 20 ]]; then
            echo -e "${YELLOW}... ($(wc -l < "$log_file") total errors, showing first 20)${NC}"
        fi
        echo -e "${CYAN}Full error log: $log_file${NC}"
    else
        print_status "info" "No obvious errors found in logs"
    fi
    echo
}

# Function to display workflow summary
display_workflow_summary() {
    print_section "WORKFLOW STATUS SUMMARY"

    local runs_json=$(get_all_runs 30)

    # Count by status
    local total=$(echo "$runs_json" | jq '. | length')
    local success=$(echo "$runs_json" | jq '[.[] | select(.conclusion == "success")] | length')
    local failure=$(echo "$runs_json" | jq '[.[] | select(.conclusion == "failure")] | length')
    local in_progress=$(echo "$runs_json" | jq '[.[] | select(.status == "in_progress")] | length')
    local cancelled=$(echo "$runs_json" | jq '[.[] | select(.conclusion == "cancelled")] | length')

    echo -e "${BOLD}Recent Runs Summary (last 30):${NC}"
    echo -e "  ${GREEN}✅ Success: $success${NC}"
    echo -e "  ${RED}❌ Failure: $failure${NC}"
    echo -e "  ${BLUE}🔄 In Progress: $in_progress${NC}"
    echo -e "  ${YELLOW}⊘  Cancelled: $cancelled${NC}"
    echo -e "  Total: $total"
    echo
}

# Function to display CI status
display_ci_status() {
    print_section "CI BUILD STATUS"

    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "ci.yml" --limit 5 \
        --json databaseId,displayTitle,headBranch,status,conclusion,createdAt,url)

    if [[ $(echo "$runs_json" | jq '. | length') -eq 0 ]]; then
        print_status "warning" "No CI runs found"
        return
    fi

    echo "$runs_json" | jq -r '.[] |
        "\(.databaseId)|\(.displayTitle)|\(.headBranch)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
    while IFS='|' read -r id title branch status conclusion created url; do
        local status_icon="⏳"
        local status_color="$YELLOW"

        if [[ "$status" == "completed" ]]; then
            if [[ "$conclusion" == "success" ]]; then
                status_icon="✅"
                status_color="$GREEN"
            else
                status_icon="❌"
                status_color="$RED"
            fi
        elif [[ "$status" == "in_progress" ]]; then
            status_icon="🔄"
            status_color="$BLUE"
        fi

        local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

        echo -e "${status_color}${status_icon} Run #$id${NC} - $title"
        echo -e "   Branch: ${CYAN}$branch${NC} | Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
        echo -e "   Created: $date_str"
        echo -e "   URL: ${BLUE}$url${NC}"

        # Show errors for failed runs
        if [[ "$conclusion" == "failure" ]]; then
            extract_errors_from_logs "$id"
        fi
        echo
    done
}

# Function to display coverage status
display_coverage_status() {
    print_section "CODE COVERAGE STATUS"

    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "coverage.yml" --limit 3 \
        --json databaseId,displayTitle,headBranch,status,conclusion,createdAt,url)

    if [[ $(echo "$runs_json" | jq '. | length') -eq 0 ]]; then
        print_status "warning" "No coverage runs found"
        return
    fi

    echo "$runs_json" | jq -r '.[] |
        "\(.databaseId)|\(.displayTitle)|\(.headBranch)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
    while IFS='|' read -r id title branch status conclusion created url; do
        local status_icon="⏳"
        local status_color="$YELLOW"

        if [[ "$status" == "completed" ]]; then
            if [[ "$conclusion" == "success" ]]; then
                status_icon="✅"
                status_color="$GREEN"
            else
                status_icon="❌"
                status_color="$RED"
            fi
        elif [[ "$status" == "in_progress" ]]; then
            status_icon="🔄"
            status_color="$BLUE"
        fi

        local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

        echo -e "${status_color}${status_icon} Run #$id${NC} - Coverage: $branch"
        echo -e "   Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
        echo -e "   Created: $date_str"

        # Try to extract coverage percentage from logs
        if [[ "$conclusion" == "success" ]]; then
            local coverage_log=$(gh run view "$id" -R "$REPO_OWNER/$REPO_NAME" --log 2>/dev/null | \
                grep -iE "coverage|percent|%" | grep -oP '\d+(\.\d+)?%' | head -n 1)
            if [[ -n "$coverage_log" ]]; then
                echo -e "   ${BLUE}📈 Coverage: $coverage_log${NC}"
            fi
        fi

        echo -e "   URL: ${BLUE}$url${NC}"
        echo
    done
}

# Function to display CodeQL status
display_codeql_status() {
    print_section "CODEQL SECURITY ANALYSIS"

    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "codeql.yml" --limit 3 \
        --json databaseId,displayTitle,headBranch,status,conclusion,createdAt,url)

    if [[ $(echo "$runs_json" | jq '. | length') -eq 0 ]]; then
        print_status "warning" "No CodeQL runs found"
        return
    fi

    echo "$runs_json" | jq -r '.[] |
        "\(.databaseId)|\(.displayTitle)|\(.headBranch)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
    while IFS='|' read -r id title branch status conclusion created url; do
        local status_icon="⏳"
        local status_color="$YELLOW"

        if [[ "$status" == "completed" ]]; then
            if [[ "$conclusion" == "success" ]]; then
                status_icon="🔒✅"
                status_color="$GREEN"
            else
                status_icon="🔒❌"
                status_color="$RED"
            fi
        elif [[ "$status" == "in_progress" ]]; then
            status_icon="🔒🔄"
            status_color="$BLUE"
        fi

        local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

        echo -e "${status_color}${status_icon} Run #$id${NC} - CodeQL: $branch"
        echo -e "   Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
        echo -e "   Created: $date_str"
        echo -e "   URL: ${BLUE}$url${NC}"
        echo
    done

    # Show CodeQL alerts
    print_status "info" "Checking for CodeQL alerts..."
    local alerts_response=$(gh api "repos/$REPO_OWNER/$REPO_NAME/code-scanning/alerts" 2>/dev/null || echo '[]')
    local alerts=$(echo "$alerts_response" | jq -r 'if type == "array" then length else 0 end' 2>/dev/null | head -1 | tr -d '\n' || echo "0")

    if [[ "$alerts" =~ ^[0-9]+$ ]] && [[ "$alerts" -gt 0 ]]; then
        print_status "warning" "Found $alerts CodeQL alert(s)"
        gh api "repos/$REPO_OWNER/$REPO_NAME/code-scanning/alerts" --jq '.[] |
            "  - [\(.rule.severity | ascii_upcase)] \(.rule.description) in \(.most_recent_instance.location.path):\(.most_recent_instance.location.start_line)"' 2>/dev/null | head -n 10
        echo
    else
        print_status "success" "No CodeQL alerts found"
    fi
}

# Function to display release status
display_release_status() {
    print_section "RELEASE BUILD STATUS"

    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "release.yml" --limit 5 \
        --json databaseId,displayTitle,headBranch,status,conclusion,createdAt,url,event)

    if [[ $(echo "$runs_json" | jq '. | length') -eq 0 ]]; then
        print_status "warning" "No release runs found"
        return
    fi

    echo "$runs_json" | jq -r '.[] |
        "\(.databaseId)|\(.displayTitle)|\(.headBranch)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)|\(.event)"' | \
    while IFS='|' read -r id title branch status conclusion created url event; do
        local status_icon="⏳"
        local status_color="$YELLOW"

        if [[ "$status" == "completed" ]]; then
            if [[ "$conclusion" == "success" ]]; then
                status_icon="🚀✅"
                status_color="$GREEN"
            else
                status_icon="🚀❌"
                status_color="$RED"
            fi
        elif [[ "$status" == "in_progress" ]]; then
            status_icon="🚀🔄"
            status_color="$BLUE"
        fi

        local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

        echo -e "${status_color}${status_icon} Run #$id${NC} - $title"
        echo -e "   Branch: ${CYAN}$branch${NC} | Event: ${PURPLE}$event${NC}"
        echo -e "   Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
        echo -e "   Created: $date_str"
        echo -e "   URL: ${BLUE}$url${NC}"

        # Show errors for failed runs
        if [[ "$conclusion" == "failure" ]]; then
            extract_errors_from_logs "$id"
        fi
        echo
    done
}

# Function to display Dependabot status
display_dependabot_status() {
    print_section "DEPENDABOT & DEPENDENCY REVIEW"

    # Show dependency review workflow
    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "dependency-review.yml" --limit 3 \
        --json databaseId,displayTitle,status,conclusion,createdAt,url 2>/dev/null || echo "[]")

    if [[ $(echo "$runs_json" | jq '. | length') -gt 0 ]]; then
        echo -e "${BOLD}Dependency Review Workflow:${NC}"
        echo "$runs_json" | jq -r '.[] |
            "\(.databaseId)|\(.displayTitle)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
        while IFS='|' read -r id title status conclusion created url; do
            local status_icon="⏳"
            local status_color="$YELLOW"

            if [[ "$status" == "completed" ]]; then
                if [[ "$conclusion" == "success" ]]; then
                    status_icon="✅"
                    status_color="$GREEN"
                else
                    status_icon="❌"
                    status_color="$RED"
                fi
            elif [[ "$status" == "in_progress" ]]; then
                status_icon="🔄"
                status_color="$BLUE"
            fi

            local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

            echo -e "  ${status_color}${status_icon} Run #$id${NC} - $title"
            echo -e "     Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
            echo -e "     Created: $date_str"
            echo
        done
    fi

    # Show Dependabot alerts
    print_status "info" "Checking for Dependabot alerts..."
    local alerts_response=$(gh api "repos/$REPO_OWNER/$REPO_NAME/dependabot/alerts" 2>/dev/null || echo '[]')
    local alerts=$(echo "$alerts_response" | jq -r 'if type == "array" then length else 0 end' 2>/dev/null | head -1 | tr -d '\n' || echo "0")

    if [[ "$alerts" =~ ^[0-9]+$ ]] && [[ "$alerts" -gt 0 ]]; then
        print_status "warning" "Found $alerts Dependabot alert(s)"
        gh api "repos/$REPO_OWNER/$REPO_NAME/dependabot/alerts" --jq '.[] |
            "  - [\(.security_advisory.severity | ascii_upcase)] \(.security_advisory.summary) - \(.dependency.package.name)"' 2>/dev/null | head -n 10
        echo
        echo -e "View all alerts: ${BLUE}https://github.com/$REPO_OWNER/$REPO_NAME/security/dependabot${NC}"
    else
        print_status "success" "No Dependabot alerts found"
    fi
    echo
}

# Function to display static analysis status
display_static_analysis_status() {
    print_section "STATIC ANALYSIS & SANITIZERS"

    # Static analysis
    echo -e "${BOLD}Static Analysis:${NC}"
    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "static-analysis.yml" --limit 3 \
        --json databaseId,displayTitle,status,conclusion,createdAt,url 2>/dev/null || echo "[]")

    if [[ $(echo "$runs_json" | jq '. | length') -gt 0 ]]; then
        echo "$runs_json" | jq -r '.[] |
            "\(.databaseId)|\(.displayTitle)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
        while IFS='|' read -r id title status conclusion created url; do
            local status_icon="⏳"
            local status_color="$YELLOW"

            if [[ "$status" == "completed" ]]; then
                if [[ "$conclusion" == "success" ]]; then
                    status_icon="✅"
                    status_color="$GREEN"
                else
                    status_icon="❌"
                    status_color="$RED"
                fi
            elif [[ "$status" == "in_progress" ]]; then
                status_icon="🔄"
                status_color="$BLUE"
            fi

            local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

            echo -e "  ${status_color}${status_icon} Run #$id${NC} - $title"
            echo -e "     Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"

            if [[ "$conclusion" == "failure" ]]; then
                # Extract specific warnings/errors
                local warnings=$(gh run view "$id" -R "$REPO_OWNER/$REPO_NAME" --log 2>/dev/null | \
                    grep -i "warning:" | wc -l)
                echo -e "     ${YELLOW}⚠️  Warnings found: $warnings${NC}"
            fi
            echo
        done
    else
        print_status "info" "No static analysis runs found"
    fi

    # Sanitizers
    echo -e "${BOLD}Sanitizers (ASan, UBSan, etc.):${NC}"
    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "sanitizers.yml" --limit 3 \
        --json databaseId,displayTitle,status,conclusion,createdAt,url 2>/dev/null || echo "[]")

    if [[ $(echo "$runs_json" | jq '. | length') -gt 0 ]]; then
        echo "$runs_json" | jq -r '.[] |
            "\(.databaseId)|\(.displayTitle)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
        while IFS='|' read -r id title status conclusion created url; do
            local status_icon="⏳"
            local status_color="$YELLOW"

            if [[ "$status" == "completed" ]]; then
                if [[ "$conclusion" == "success" ]]; then
                    status_icon="✅"
                    status_color="$GREEN"
                else
                    status_icon="❌"
                    status_color="$RED"
                fi
            elif [[ "$status" == "in_progress" ]]; then
                status_icon="🔄"
                status_color="$BLUE"
            fi

            local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

            echo -e "  ${status_color}${status_icon} Run #$id${NC} - $title"
            echo -e "     Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"

            if [[ "$conclusion" == "failure" ]]; then
                extract_errors_from_logs "$id"
            fi
            echo
        done
    else
        print_status "info" "No sanitizer runs found"
    fi
}

# Function to display benchmarks status
display_benchmarks_status() {
    print_section "BENCHMARKS"

    local runs_json=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "benchmarks.yml" --limit 3 \
        --json databaseId,displayTitle,status,conclusion,createdAt,url 2>/dev/null || echo "[]")

    if [[ $(echo "$runs_json" | jq '. | length') -eq 0 ]]; then
        print_status "info" "No benchmark runs found"
        return
    fi

    echo "$runs_json" | jq -r '.[] |
        "\(.databaseId)|\(.displayTitle)|\(.status)|\(.conclusion // "N/A")|\(.createdAt)|\(.url)"' | \
    while IFS='|' read -r id title status conclusion created url; do
        local status_icon="⏳"
        local status_color="$YELLOW"

        if [[ "$status" == "completed" ]]; then
            if [[ "$conclusion" == "success" ]]; then
                status_icon="📊✅"
                status_color="$GREEN"
            else
                status_icon="📊❌"
                status_color="$RED"
            fi
        elif [[ "$status" == "in_progress" ]]; then
            status_icon="📊🔄"
            status_color="$BLUE"
        fi

        local date_str=$(date -d "$created" +"%Y-%m-%d %H:%M" 2>/dev/null || echo "$created")

        echo -e "${status_color}${status_icon} Run #$id${NC} - $title"
        echo -e "   Status: ${status_color}$status${NC} | Result: ${status_color}${conclusion}${NC}"
        echo -e "   Created: $date_str"
        echo -e "   URL: ${BLUE}$url${NC}"
        echo
    done
}

# Function to view specific run details
view_run() {
    local run_id=$1

    if [[ -z "$run_id" ]]; then
        print_status "failure" "Run ID required"
        echo "Usage: $0 view <run_id>"
        exit 1
    fi

    print_section "RUN DETAILS: #$run_id"

    # Get run details using gh run view
    gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME"

    echo
    read -p "Show full logs? (y/n): " show_logs
    if [[ "$show_logs" =~ ^[Yy] ]]; then
        print_section "RUN LOGS: #$run_id"
        gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME" --log
    fi

    echo
    read -p "Extract errors? (y/n): " extract_errors
    if [[ "$extract_errors" =~ ^[Yy] ]]; then
        extract_errors_from_logs "$run_id"
    fi
}

# Function to watch workflows in real-time
watch_workflows() {
    local interval=${1:-30}
    print_status "info" "Watching workflows (refresh every ${interval}s). Press Ctrl+C to stop."
    echo

    while true; do
        clear
        show_all_status
        echo
        print_status "info" "Last updated: $(date)"
        print_status "info" "Press Ctrl+C to stop watching"
        sleep "$interval"
    done
}

# Function to download and analyze logs
download_and_analyze_logs() {
    local workflow=${1:-""}
    local limit=${2:-5}

    print_section "DOWNLOADING AND ANALYZING LOGS"

    if [[ -z "$workflow" ]]; then
        # Download logs for failed runs from all workflows
        print_status "info" "Finding failed runs..."

        local failed_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 20 \
            --json databaseId,conclusion,workflowName | \
            jq -r '.[] | select(.conclusion == "failure") | "\(.databaseId)|\(.workflowName)"')

        if [[ -z "$failed_runs" ]]; then
            print_status "success" "No failed runs found!"
            return
        fi

        echo "$failed_runs" | while IFS='|' read -r run_id workflow_name; do
            print_status "info" "Analyzing failed run #$run_id from $workflow_name..."
            extract_errors_from_logs "$run_id"
        done
    else
        # Download logs for specific workflow
        print_status "info" "Downloading logs for $workflow..."

        local runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" -w "$workflow" --limit "$limit" \
            --json databaseId,conclusion | jq -r '.[] | "\(.databaseId)|\(.conclusion)"')

        echo "$runs" | while IFS='|' read -r run_id conclusion; do
            local log_file="$LOGS_DIR/${workflow}_${run_id}.log"
            print_status "info" "Downloading run #$run_id..."
            gh run view "$run_id" -R "$REPO_OWNER/$REPO_NAME" --log > "$log_file" 2>&1
            print_status "success" "Saved to: $log_file"

            if [[ "$conclusion" == "failure" ]]; then
                extract_errors_from_logs "$run_id"
            fi
        done
    fi
}

# Function to show all status
show_all_status() {
    display_workflow_summary
    display_ci_status
    display_coverage_status
    display_codeql_status
    display_dependabot_status
    display_static_analysis_status
    display_release_status
    display_benchmarks_status
}

# Function to list recent runs
list_runs() {
    local workflow=${1:-""}
    local limit=${2:-20}

    print_section "RECENT WORKFLOW RUNS"

    if [[ -z "$workflow" ]]; then
        gh run list -R "$REPO_OWNER/$REPO_NAME" --limit "$limit"
    else
        gh run list -R "$REPO_OWNER/$REPO_NAME" -w "$workflow" --limit "$limit"
    fi
}

# Function to show security overview
show_security_overview() {
    print_section "SECURITY OVERVIEW"

    # CodeQL alerts
    echo -e "${BOLD}CodeQL Security Alerts:${NC}"
    local codeql_alerts=$(gh api "repos/$REPO_OWNER/$REPO_NAME/code-scanning/alerts" --jq 'length' 2>/dev/null || echo "0")

    if [[ "$codeql_alerts" -gt 0 ]]; then
        print_status "warning" "Found $codeql_alerts CodeQL alert(s)"
        gh api "repos/$REPO_OWNER/$REPO_NAME/code-scanning/alerts" --jq '.[] |
            "  [\(.rule.severity | ascii_upcase)] \(.rule.description)\n    Location: \(.most_recent_instance.location.path):\(.most_recent_instance.location.start_line)\n    State: \(.state)"' 2>/dev/null
    else
        print_status "success" "No CodeQL alerts"
    fi
    echo

    # Dependabot alerts
    echo -e "${BOLD}Dependabot Alerts:${NC}"
    local dependabot_alerts=$(gh api "repos/$REPO_OWNER/$REPO_NAME/dependabot/alerts" --jq 'length' 2>/dev/null || echo "0")

    if [[ "$dependabot_alerts" -gt 0 ]]; then
        print_status "warning" "Found $dependabot_alerts Dependabot alert(s)"
        gh api "repos/$REPO_OWNER/$REPO_NAME/dependabot/alerts" --jq '.[] |
            "  [\(.security_advisory.severity | ascii_upcase)] \(.security_advisory.summary)\n    Package: \(.dependency.package.name)\n    State: \(.state)"' 2>/dev/null
    else
        print_status "success" "No Dependabot alerts"
    fi
    echo

    # Security link
    echo -e "${BLUE}Full security dashboard: https://github.com/$REPO_OWNER/$REPO_NAME/security${NC}"
}

# Function to cancel running workflows
cancel_running_workflows() {
    print_section "CANCELING RUNNING WORKFLOWS"

    # Get all running workflows
    print_status "info" "Finding running workflows..."
    local running_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 50 \
        --json databaseId,displayTitle,status,workflowName | \
        jq -r '.[] | select(.status == "in_progress" or .status == "queued") | "\(.databaseId)|\(.workflowName)|\(.displayTitle)"')

    if [[ -z "$running_runs" ]]; then
        print_status "success" "No running workflows to cancel"
        return
    fi

    local count=$(echo "$running_runs" | wc -l)
    print_status "warning" "Found $count running/queued workflow(s)"
    echo

    # Show what will be cancelled
    echo -e "${BOLD}Workflows to cancel:${NC}"
    echo "$running_runs" | while IFS='|' read -r id workflow title; do
        echo -e "  ${YELLOW}⏳${NC} Run #$id - $workflow"
        echo -e "     $title"
    done
    echo

    # Confirm
    read -p "Cancel all these workflows? (y/N): " confirm
    if [[ ! "$confirm" =~ ^[Yy] ]]; then
        print_status "info" "Cancelled operation"
        return
    fi

    # Cancel each workflow
    echo
    print_status "info" "Cancelling workflows..."
    local cancelled=0
    local failed=0

    echo "$running_runs" | while IFS='|' read -r id workflow title; do
        if gh run cancel "$id" -R "$REPO_OWNER/$REPO_NAME" 2>/dev/null; then
            print_status "success" "Cancelled run #$id"
            ((cancelled++))
        else
            print_status "failure" "Failed to cancel run #$id"
            ((failed++))
        fi
    done

    echo
    print_status "success" "Cancellation complete!"
    echo -e "  Cancelled: $cancelled"
    if [[ $failed -gt 0 ]]; then
        echo -e "  ${RED}Failed: $failed${NC}"
    fi
}

# Function to cancel workflows by pattern
cancel_workflows_by_pattern() {
    local pattern=$1

    if [[ -z "$pattern" ]]; then
        print_status "failure" "Pattern required"
        echo "Usage: $0 cancel-pattern <workflow-name-pattern>"
        echo "Examples:"
        echo "  $0 cancel-pattern ci.yml"
        echo "  $0 cancel-pattern 'dependabot'"
        exit 1
    fi

    print_section "CANCELING WORKFLOWS: $pattern"

    # Get matching running workflows
    print_status "info" "Finding workflows matching '$pattern'..."
    local running_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 50 \
        --json databaseId,displayTitle,status,workflowName | \
        jq -r --arg pattern "$pattern" \
        '.[] | select((.status == "in_progress" or .status == "queued") and (.workflowName | contains($pattern) or .displayTitle | contains($pattern))) | "\(.databaseId)|\(.workflowName)|\(.displayTitle)"')

    if [[ -z "$running_runs" ]]; then
        print_status "info" "No matching running workflows found"
        return
    fi

    local count=$(echo "$running_runs" | wc -l)
    print_status "warning" "Found $count matching workflow(s)"
    echo

    # Show what will be cancelled
    echo -e "${BOLD}Workflows to cancel:${NC}"
    echo "$running_runs" | while IFS='|' read -r id workflow title; do
        echo -e "  ${YELLOW}⏳${NC} Run #$id - $workflow"
        echo -e "     $title"
    done
    echo

    # Confirm
    read -p "Cancel these workflows? (y/N): " confirm
    if [[ ! "$confirm" =~ ^[Yy] ]]; then
        print_status "info" "Cancelled operation"
        return
    fi

    # Cancel each workflow
    echo
    print_status "info" "Cancelling workflows..."
    echo "$running_runs" | while IFS='|' read -r id workflow title; do
        if gh run cancel "$id" -R "$REPO_OWNER/$REPO_NAME" 2>/dev/null; then
            print_status "success" "Cancelled run #$id"
        else
            print_status "failure" "Failed to cancel run #$id"
        fi
    done

    echo
    print_status "success" "Done!"
}

# Function to delete stopped workflows
delete_stopped_workflows() {
    print_section "DELETE STOPPED WORKFLOWS"

    print_status "info" "Finding stopped (failed/cancelled) workflows..."
    local stopped_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 100 \
        --json databaseId,displayTitle,conclusion,status,workflowName | \
        jq -r '.[] | select(.conclusion == "failure" or .conclusion == "cancelled") | "\(.databaseId)|\(.workflowName)|\(.displayTitle)|\(.conclusion)"')

    if [[ -z "$stopped_runs" ]]; then
        print_status "success" "No stopped workflows to delete"
        return
    fi

    local count=$(echo "$stopped_runs" | wc -l)
    print_status "warning" "Found $count stopped workflow(s)"
    echo

    # Show what will be deleted
    echo -e "${BOLD}Workflows to DELETE from history:${NC}"
    echo "$stopped_runs" | while IFS='|' read -r id workflow title conclusion; do
        if [[ "$conclusion" == "failure" ]]; then
            echo -e "  ${RED}❌ Failed${NC} - Run #$id - $workflow"
        else
            echo -e "  ${YELLOW}⊘  Cancelled${NC} - Run #$id - $workflow"
        fi
        echo -e "     $title"
    done
    echo

    print_status "warning" "This will PERMANENTLY delete these runs from GitHub history!"
    read -p "Delete all these workflows? (y/N): " confirm
    if [[ ! "$confirm" =~ ^[Yy] ]]; then
        print_status "info" "Cancelled operation"
        return
    fi

    # Delete all workflows in batch (10 at a time in parallel)
    echo
    print_status "info" "Deleting $count workflows in batch..."

    # Extract all IDs and delete in parallel
    local ids=$(echo "$stopped_runs" | cut -d'|' -f1)
    echo "$ids" | xargs -P 10 -I {} sh -c 'gh run delete {} -R '"$REPO_OWNER/$REPO_NAME"' 2>/dev/null && echo "✓ Deleted {}" || echo "✗ Failed {}"'

    echo
    print_status "success" "Deletion complete! Processed $count workflow(s)"
}

# Function to delete workflows by pattern
delete_workflows_by_pattern() {
    local pattern=$1

    if [[ -z "$pattern" ]]; then
        print_status "failure" "Pattern required"
        echo "Usage: $0 delete-pattern <workflow-name-pattern>"
        echo "Examples:"
        echo "  $0 delete-pattern ci.yml"
        echo "  $0 delete-pattern 'dependabot'"
        exit 1
    fi

    print_section "DELETE WORKFLOWS: $pattern"

    # Get matching stopped workflows
    print_status "info" "Finding stopped workflows matching '$pattern'..."
    local stopped_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 100 \
        --json databaseId,displayTitle,conclusion,workflowName | \
        jq -r --arg pattern "$pattern" \
        '.[] | select((.conclusion == "failure" or .conclusion == "cancelled") and (.workflowName | contains($pattern) or .displayTitle | contains($pattern))) | "\(.databaseId)|\(.workflowName)|\(.displayTitle)|\(.conclusion)"')

    if [[ -z "$stopped_runs" ]]; then
        print_status "info" "No matching stopped workflows found"
        return
    fi

    local count=$(echo "$stopped_runs" | wc -l)
    print_status "warning" "Found $count matching workflow(s)"
    echo

    # Show what will be deleted
    echo -e "${BOLD}Workflows to DELETE:${NC}"
    echo "$stopped_runs" | while IFS='|' read -r id workflow title conclusion; do
        if [[ "$conclusion" == "failure" ]]; then
            echo -e "  ${RED}❌ Failed${NC} - Run #$id - $workflow"
        else
            echo -e "  ${YELLOW}⊘  Cancelled${NC} - Run #$id - $workflow"
        fi
        echo -e "     $title"
    done
    echo

    print_status "warning" "This will PERMANENTLY delete these runs!"
    read -p "Delete these workflows? (y/N): " confirm
    if [[ ! "$confirm" =~ ^[Yy] ]]; then
        print_status "info" "Cancelled operation"
        return
    fi

    # Delete all workflows in batch (10 at a time in parallel)
    echo
    print_status "info" "Deleting $count workflows in batch..."

    # Extract all IDs and delete in parallel
    local ids=$(echo "$stopped_runs" | cut -d'|' -f1)
    echo "$ids" | xargs -P 10 -I {} sh -c 'gh run delete {} -R '"$REPO_OWNER/$REPO_NAME"' 2>/dev/null && echo "✓ Deleted {}" || echo "✗ Failed {}"'

    echo
    print_status "success" "Deletion complete! Processed $count workflow(s)"
}

# Function to delete all workflows except those from the latest commit
delete_old_workflows() {
    print_section "DELETE OLD WORKFLOWS (Keep Latest Commit Only)"

    # Get the latest commit SHA
    local latest_commit=$(git rev-parse HEAD 2>/dev/null | cut -c1-7)
    if [[ -z "$latest_commit" ]]; then
        print_status "failure" "Could not determine latest commit. Are you in a git repository?"
        exit 1
    fi

    local latest_commit_full=$(git rev-parse HEAD 2>/dev/null)
    print_status "info" "Latest commit: ${CYAN}$latest_commit${NC} ($(git log -1 --format=%s HEAD 2>/dev/null))"
    echo

    # Get all workflow runs NOT from the latest commit
    print_status "info" "Finding workflows NOT from the latest commit..."
    local old_runs=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 200 \
        --json databaseId,displayTitle,headSha,workflowName,conclusion,status | \
        jq -r --arg commit "$latest_commit_full" \
        '.[] | select(.headSha != $commit) | "\(.databaseId)|\(.workflowName)|\(.displayTitle)|\(.conclusion // .status)|\(.headSha[0:7])"')

    if [[ -z "$old_runs" ]]; then
        print_status "success" "No old workflows to delete! All runs are from the latest commit."
        return
    fi

    local count=$(echo "$old_runs" | wc -l)
    print_status "warning" "Found $count workflow(s) from older commits"
    echo

    # Show what will be deleted (first 20)
    echo -e "${BOLD}Workflows to DELETE (showing first 20 of $count):${NC}"
    echo "$old_runs" | head -20 | while IFS='|' read -r id workflow title conclusion commit_sha; do
        local icon="📋"
        local color="$NC"
        case "$conclusion" in
            "success") icon="${GREEN}✅"; color="$GREEN" ;;
            "failure") icon="${RED}❌"; color="$RED" ;;
            "cancelled") icon="${YELLOW}⊘"; color="$YELLOW" ;;
            "in_progress") icon="${BLUE}🔄"; color="$BLUE" ;;
        esac
        echo -e "  ${icon}${NC} Run #$id - $workflow (commit: ${CYAN}$commit_sha${NC})"
        echo -e "     $title"
    done
    
    if [[ $count -gt 20 ]]; then
        echo -e "  ${YELLOW}... and $((count - 20)) more${NC}"
    fi
    echo

    echo -e "${BOLD}${CYAN}KEEPING:${NC} Workflows from commit ${CYAN}$latest_commit${NC}"
    local keep_count=$(gh run list -R "$REPO_OWNER/$REPO_NAME" --limit 200 \
        --json databaseId,headSha | \
        jq -r --arg commit "$latest_commit_full" \
        '[.[] | select(.headSha == $commit)] | length')
    echo -e "  → $keep_count workflow(s) from latest commit will be preserved"
    echo

    print_status "warning" "This will PERMANENTLY delete $count workflow runs from older commits!"
    read -p "Delete all old workflows? (y/N): " confirm
    if [[ ! "$confirm" =~ ^[Yy] ]]; then
        print_status "info" "Cancelled operation"
        return
    fi

    # Delete all old workflows in batch (10 at a time in parallel)
    echo
    print_status "info" "Deleting $count workflows in batch..."

    # Extract all IDs and delete in parallel
    local ids=$(echo "$old_runs" | cut -d'|' -f1)
    echo "$ids" | xargs -P 10 -I {} sh -c 'gh run delete {} -R '"$REPO_OWNER/$REPO_NAME"' 2>/dev/null && echo "✓ Deleted {}" || echo "✗ Failed {}"'

    echo
    print_status "success" "Deletion complete! Processed $count workflow(s)"
    echo -e "${GREEN}✅ Kept $keep_count workflow(s) from latest commit${NC}"
}

# Function to show help
show_help() {
    cat << 'EOF'
╔═══════════════════════════════════════════════════════════════════════╗
║           MetaImGUI GitHub Actions Monitoring Tool                    ║
║                     Enhanced Edition                                  ║
╚═══════════════════════════════════════════════════════════════════════╝

USAGE:
    ./scripts/monitor_actions.sh [COMMAND] [OPTIONS]

COMMANDS:
    status              Show comprehensive status of all workflows (default)
    watch [seconds]     Watch workflows in real-time (default: 30s refresh)
    list [workflow]     List recent runs (optionally for specific workflow)
    view <run_id>       View detailed information for a specific run
    logs [workflow]     Download and analyze logs (all failed or specific workflow)
    security            Show security overview (CodeQL + Dependabot)
    ci                  Show CI build status only
    coverage            Show coverage status only
    codeql              Show CodeQL status only
    release             Show release build status only
    cancel              Cancel all running/queued workflows (interactive)
    cancel-pattern <p>  Cancel workflows matching pattern (e.g., 'dependabot')
    delete              Delete all stopped (failed/cancelled) workflows
    delete-pattern <p>  Delete stopped workflows matching pattern
    delete-old          Delete ALL workflows except those from the latest commit
    help                Show this help message

EXAMPLES:
    # Show full status dashboard
    ./scripts/monitor_actions.sh
    ./scripts/monitor_actions.sh status

    # Watch workflows with auto-refresh
    ./scripts/monitor_actions.sh watch
    ./scripts/monitor_actions.sh watch 10      # Refresh every 10 seconds

    # List recent runs
    ./scripts/monitor_actions.sh list
    ./scripts/monitor_actions.sh list ci.yml   # List CI runs only

    # View specific run details
    ./scripts/monitor_actions.sh view 1234567890

    # Download and analyze logs
    ./scripts/monitor_actions.sh logs          # All failed runs
    ./scripts/monitor_actions.sh logs ci.yml   # Specific workflow

    # Security overview
    ./scripts/monitor_actions.sh security

    # Individual workflow status
    ./scripts/monitor_actions.sh ci
    ./scripts/monitor_actions.sh coverage
    ./scripts/monitor_actions.sh codeql

    # Cancel workflows (SAVE CREDITS!)
    ./scripts/monitor_actions.sh cancel                    # Cancel all running
    ./scripts/monitor_actions.sh cancel-pattern dependabot # Cancel Dependabot runs
    ./scripts/monitor_actions.sh cancel-pattern ci.yml     # Cancel CI runs

    # Delete stopped workflows (CLEAN HISTORY!)
    ./scripts/monitor_actions.sh delete                    # Delete all failed/cancelled
    ./scripts/monitor_actions.sh delete-pattern dependabot # Delete Dependabot runs only
    ./scripts/monitor_actions.sh delete-pattern ci.yml     # Delete CI failures only
    ./scripts/monitor_actions.sh delete-old                # Delete all except latest commit

FEATURES:
    ✅ Comprehensive workflow monitoring (CI, Release, Coverage, CodeQL, etc.)
    ✅ Real-time status updates with color-coded output
    ✅ Automatic error extraction from failed runs
    ✅ Code coverage percentage tracking
    ✅ Security alerts (CodeQL + Dependabot)
    ✅ Direct gh CLI integration for detailed views
    ✅ Log download and analysis
    ✅ Dependency review monitoring
    ✅ Static analysis and sanitizer status

REQUIREMENTS:
    - gh (GitHub CLI) - REQUIRED
    - jq (JSON processor)
    - Standard Unix tools (grep, sed, awk)

INSTALLATION:
    Ubuntu/Debian:
        sudo apt update
        sudo apt install gh jq

    macOS:
        brew install gh jq

    Authentication:
        gh auth login

WORKFLOWS MONITORED:
    • CI Builds (ci.yml)
    • Release Builds (release.yml)
    • Code Coverage (coverage.yml)
    • CodeQL Security Analysis (codeql.yml)
    • Dependency Review (dependency-review.yml)
    • Static Analysis (static-analysis.yml)
    • Sanitizers (sanitizers.yml)
    • Benchmarks (benchmarks.yml)
    • Documentation (docs.yml)

LOG FILES:
    Logs are saved to: ./workflow_logs/
    Error extracts: ./workflow_logs/run_<id>_errors.txt

CACHE:
    Configuration: ~/.metaimgui_monitor_config
    Cache directory: ~/.cache/metaimgui_monitor

For more information, visit:
    https://github.com/YOUR_REPO/wiki/Monitoring

EOF
}

# Main script logic
main() {
    local command=${1:-status}
    shift || true

    # Check dependencies
    if ! command -v jq &> /dev/null; then
        print_status "failure" "jq is not installed. Please install it:"
        echo "  Ubuntu/Debian: sudo apt install jq"
        echo "  macOS: brew install jq"
        exit 1
    fi

    case $command in
        "help"|"-h"|"--help")
            show_help
            exit 0
            ;;
        "status")
            load_repository
            check_and_setup_gh
            show_all_status
            ;;
        "watch")
            local interval=${1:-30}
            load_repository
            check_and_setup_gh
            watch_workflows "$interval"
            ;;
        "list")
            local workflow=$1
            local limit=${2:-20}
            load_repository
            check_and_setup_gh
            list_runs "$workflow" "$limit"
            ;;
        "view")
            local run_id=$1
            load_repository
            check_and_setup_gh
            view_run "$run_id"
            ;;
        "logs")
            local workflow=$1
            local limit=${2:-5}
            load_repository
            check_and_setup_gh
            download_and_analyze_logs "$workflow" "$limit"
            ;;
        "security")
            load_repository
            check_and_setup_gh
            show_security_overview
            ;;
        "ci")
            load_repository
            check_and_setup_gh
            display_ci_status
            ;;
        "coverage")
            load_repository
            check_and_setup_gh
            display_coverage_status
            ;;
        "codeql")
            load_repository
            check_and_setup_gh
            display_codeql_status
            ;;
        "release")
            load_repository
            check_and_setup_gh
            display_release_status
            ;;
        "dependabot")
            load_repository
            check_and_setup_gh
            display_dependabot_status
            ;;
        "static-analysis")
            load_repository
            check_and_setup_gh
            display_static_analysis_status
            ;;
        "benchmarks")
            load_repository
            check_and_setup_gh
            display_benchmarks_status
            ;;
        "cancel")
            load_repository
            check_and_setup_gh
            cancel_running_workflows
            ;;
        "cancel-pattern")
            local pattern=$1
            load_repository
            check_and_setup_gh
            cancel_workflows_by_pattern "$pattern"
            ;;
        "delete")
            load_repository
            check_and_setup_gh
            delete_stopped_workflows
            ;;
        "delete-pattern")
            local pattern=$1
            load_repository
            check_and_setup_gh
            delete_workflows_by_pattern "$pattern"
            ;;
        "delete-old")
            load_repository
            check_and_setup_gh
            delete_old_workflows
            ;;
        *)
            print_status "failure" "Unknown command: $command"
            echo
            echo "Run '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
