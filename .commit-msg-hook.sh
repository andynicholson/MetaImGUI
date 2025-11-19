#!/bin/bash
# Commit message validation hook for Conventional Commits
# Install with: cp .commit-msg-hook.sh .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg

COMMIT_MSG_FILE=$1
COMMIT_MSG=$(cat "$COMMIT_MSG_FILE")

# Skip merge commits
if echo "$COMMIT_MSG" | head -1 | grep -qE "^Merge (branch|pull request)"; then
    exit 0
fi

# Skip revert commits
if echo "$COMMIT_MSG" | head -1 | grep -qE "^Revert "; then
    exit 0
fi

# Valid types
VALID_TYPES="feat|fix|docs|style|refactor|perf|test|build|ci|chore|revert"

# Check format: type(optional-scope): description
if ! echo "$COMMIT_MSG" | head -1 | grep -qE "^($VALID_TYPES)(\(.+\))?!?: .+"; then
    echo "❌ Invalid commit message format!"
    echo ""
    echo "Commit message must follow Conventional Commits format:"
    echo "  <type>(<scope>): <description>"
    echo ""
    echo "Valid types: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert"
    echo ""
    echo "Examples:"
    echo "  feat: add dark mode support"
    echo "  fix(logger): prevent memory leak"
    echo "  docs: update README with examples"
    echo "  feat!: breaking API change"
    echo ""
    echo "Your commit message:"
    echo "  $COMMIT_MSG" | head -1
    echo ""
    exit 1
fi

# Check that description is not too short
DESCRIPTION=$(echo "$COMMIT_MSG" | head -1 | sed -E "s/^($VALID_TYPES)(\(.+\))?!?: //")
if [ ${#DESCRIPTION} -lt 10 ]; then
    echo "❌ Commit description is too short (minimum 10 characters)"
    echo ""
    echo "Your description: '$DESCRIPTION' (${#DESCRIPTION} characters)"
    echo ""
    exit 1
fi

# Check that first line is not too long
FIRST_LINE=$(echo "$COMMIT_MSG" | head -1)
if [ ${#FIRST_LINE} -gt 100 ]; then
    echo "⚠️  Warning: Commit message first line is longer than 100 characters (${#FIRST_LINE})"
    echo "Consider keeping it under 72 characters for better readability"
    echo ""
fi

exit 0

