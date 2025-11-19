#!/bin/bash

# Quick Status Script - MetaImGUI Edition
# Provides direct URLs to GitHub Actions without API dependencies

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

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
}

# Main script
main() {
    print_status "header" "MetaImGUI GitHub Actions Quick Status"
    echo
    
    # Detect repository
    detect_repository
    
    echo
    print_status "info" "Direct Links to GitHub Actions:"
    echo
    
    # Main Actions page
    echo -e "${BLUE}📋 Main Actions:${NC}"
    echo "  https://github.com/$REPO_OWNER/$REPO_NAME/actions"
    echo
    
    # Specific workflows
    echo -e "${BLUE}🔧 CI Workflow:${NC}"
    echo "  https://github.com/$REPO_OWNER/$REPO_NAME/actions/workflows/ci.yml"
    echo
    
    echo -e "${BLUE}🚀 Release Workflow:${NC}"
    echo "  https://github.com/$REPO_OWNER/$REPO_NAME/actions/workflows/release.yml"
    echo
    
    # Repository management
    echo -e "${BLUE}⚙️  Repository Management:${NC}"
    echo "  Settings: https://github.com/$REPO_OWNER/$REPO_NAME/settings/actions"
    echo "  Secrets: https://github.com/$REPO_OWNER/$REPO_NAME/settings/secrets/actions"
    echo
    
    print_status "info" "Quick Troubleshooting:"
    echo "  1. Check if workflows are enabled in repository settings"
    echo "  2. Verify branch protection rules if using protected branches"
    echo "  3. Check workflow file syntax in .github/workflows/"
    echo "  4. Review recent commits for trigger conditions"
    echo
    
    print_status "info" "For detailed monitoring, use:"
    echo "  ./scripts/monitor_actions.sh"
    echo "  ./scripts/monitor_actions.sh watch"
    echo
}

# Run main function
main "$@" 