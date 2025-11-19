#!/bin/bash

# Release Preparation Script for MetaImGUI
# Automates version bumping, changelog updates, and release tagging

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m'

# Get to repo root
cd "$(git rev-parse --show-toplevel)"

echo -e "${BOLD}${BLUE}MetaImGUI Release Preparation${NC}"
echo "================================"
echo

# Get current version from latest tag
CURRENT_VERSION=$(git tag --sort=-v:refname | head -1)
if [[ -z "$CURRENT_VERSION" ]]; then
    echo -e "${RED}No tags found. Using v0.0.0 as starting point.${NC}"
    CURRENT_VERSION="v0.0.0"
fi

echo -e "Current version: ${GREEN}${CURRENT_VERSION}${NC}"
echo

# Parse version numbers
CURRENT_VERSION_NUM=${CURRENT_VERSION#v}
IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT_VERSION_NUM"

# Calculate next versions
NEXT_PATCH="v${MAJOR}.${MINOR}.$((PATCH + 1))"
NEXT_MINOR="v${MAJOR}.$((MINOR + 1)).0"
NEXT_MAJOR="v$((MAJOR + 1)).0.0"

# Suggest version based on commits
COMMITS_SINCE=$(git log ${CURRENT_VERSION}..HEAD --oneline 2>/dev/null | wc -l)
if [[ $COMMITS_SINCE -eq 0 ]]; then
    echo -e "${YELLOW}No commits since ${CURRENT_VERSION}${NC}"
    echo "Nothing to release!"
    exit 1
fi

echo -e "${BLUE}Commits since ${CURRENT_VERSION}: ${COMMITS_SINCE}${NC}"
echo

# Analyze commits to suggest version
HAS_BREAKING=$(git log ${CURRENT_VERSION}..HEAD --oneline | grep -iE "(BREAKING|breaking change)" | wc -l)
HAS_FEAT=$(git log ${CURRENT_VERSION}..HEAD --oneline | grep -iE "^[a-f0-9]+ feat" | wc -l)
HAS_FIX=$(git log ${CURRENT_VERSION}..HEAD --oneline | grep -iE "^[a-f0-9]+ fix" | wc -l)

if [[ $HAS_BREAKING -gt 0 ]]; then
    SUGGESTED=$NEXT_MAJOR
    REASON="(breaking changes detected)"
elif [[ $HAS_FEAT -gt 0 ]]; then
    SUGGESTED=$NEXT_MINOR
    REASON="(new features detected)"
else
    SUGGESTED=$NEXT_PATCH
    REASON="(bug fixes only)"
fi

echo -e "${BOLD}Version suggestions:${NC}"
echo -e "  ${GREEN}${NEXT_PATCH}${NC} - Patch (bug fixes)"
echo -e "  ${GREEN}${NEXT_MINOR}${NC} - Minor (new features)"
echo -e "  ${GREEN}${NEXT_MAJOR}${NC} - Major (breaking changes)"
echo
echo -e "Suggested: ${YELLOW}${SUGGESTED}${NC} ${REASON}"
echo

# Get version from user
read -p "Enter version to release [${SUGGESTED}]: " NEW_VERSION
NEW_VERSION=${NEW_VERSION:-$SUGGESTED}

# Ensure v prefix
if [[ ! "$NEW_VERSION" =~ ^v ]]; then
    NEW_VERSION="v${NEW_VERSION}"
fi

echo
echo -e "${BOLD}Preparing release: ${GREEN}${NEW_VERSION}${NC}"
echo

# Generate changelog entry
echo -e "${BLUE}Generating changelog...${NC}"

TEMP_CHANGELOG=$(mktemp)
DATE=$(date +%Y-%m-%d)

# Get commit summary by type
FEATURES=$(git log ${CURRENT_VERSION}..HEAD --oneline --no-merges | grep -E "^[a-f0-9]+ feat" | sed 's/^[a-f0-9]* feat[:(]*[a-z]*[):]*/- /' || echo "")
FIXES=$(git log ${CURRENT_VERSION}..HEAD --oneline --no-merges | grep -E "^[a-f0-9]+ fix" | sed 's/^[a-f0-9]* fix[:(]*[a-z]*[):]*/- /' || echo "")
CHORES=$(git log ${CURRENT_VERSION}..HEAD --oneline --no-merges | grep -E "^[a-f0-9]+ (chore|docs|ci|refactor)" | sed 's/^[a-f0-9]* [a-z]*[:(]*[a-z]*[):]*/- /' || echo "")

# Build new changelog entry
cat > "$TEMP_CHANGELOG" << EOF
## [${NEW_VERSION#v}] - ${DATE}

EOF

if [[ -n "$FEATURES" ]]; then
    echo "### Added" >> "$TEMP_CHANGELOG"
    echo "$FEATURES" >> "$TEMP_CHANGELOG"
    echo >> "$TEMP_CHANGELOG"
fi

if [[ -n "$FIXES" ]]; then
    echo "### Fixed" >> "$TEMP_CHANGELOG"
    echo "$FIXES" >> "$TEMP_CHANGELOG"
    echo >> "$TEMP_CHANGELOG"
fi

if [[ -n "$CHORES" ]]; then
    echo "### Changed" >> "$TEMP_CHANGELOG"
    echo "$CHORES" >> "$TEMP_CHANGELOG"
    echo >> "$TEMP_CHANGELOG"
fi

# Preview changelog
echo -e "${BOLD}New changelog entry:${NC}"
echo "---"
cat "$TEMP_CHANGELOG"
echo "---"
echo

read -p "Add this to CHANGELOG.md? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted."
    rm "$TEMP_CHANGELOG"
    exit 1
fi

# Update CHANGELOG.md - insert after [Unreleased] section
if grep -q "## \[Unreleased\]" CHANGELOG.md; then
    # Find line number of [Unreleased] section end (first blank line after the header blocks)
    LINE=$(awk '/## \[Unreleased\]/,/^## \[/ {print NR; if(/^$/ && prev ~ /^###/) exit} {prev=$0}' CHANGELOG.md | tail -1)

    # Insert new entry
    {
        head -n "$LINE" CHANGELOG.md
        cat "$TEMP_CHANGELOG"
        tail -n +$((LINE + 1)) CHANGELOG.md
    } > CHANGELOG.md.tmp
    mv CHANGELOG.md.tmp CHANGELOG.md
else
    # Just insert after header
    {
        head -n 7 CHANGELOG.md
        cat "$TEMP_CHANGELOG"
        tail -n +8 CHANGELOG.md
    } > CHANGELOG.md.tmp
    mv CHANGELOG.md.tmp CHANGELOG.md
fi

rm "$TEMP_CHANGELOG"
echo -e "${GREEN}✓ Updated CHANGELOG.md${NC}"

# Note: CMakeLists.txt uses git-based versioning via GetGitVersion.cmake
# No need to manually update version - it's extracted from git tags automatically

# Update version in README badges if present
if grep -q "shields.io.*version.*v[0-9]" README.md; then
    sed -i "s/\(shields.io.*version.*\)v[0-9.]*\(.*\)/\1${NEW_VERSION}\2/" README.md
    echo -e "${GREEN}✓ Updated README.md badges${NC}"
fi

# Stage changes
git add CHANGELOG.md
[[ -f CMakeLists.txt ]] && git add CMakeLists.txt
git add README.md 2>/dev/null || true

echo
echo -e "${BOLD}Files staged for commit:${NC}"
git diff --cached --stat
echo

# Generate commit message
COMMIT_MSG="chore: release ${NEW_VERSION}

Release ${NEW_VERSION}

Changes since ${CURRENT_VERSION}:
- ${COMMITS_SINCE} commits
- ${HAS_FEAT} new features
- ${HAS_FIX} bug fixes

See CHANGELOG.md for full details."

echo -e "${BOLD}Commit message:${NC}"
echo "---"
echo "$COMMIT_MSG"
echo "---"
echo

read -p "Create commit? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted. Changes staged but not committed."
    exit 1
fi

# Commit
git commit -m "$COMMIT_MSG"
echo -e "${GREEN}✓ Created release commit${NC}"

# Generate tag message
TAG_MSG="Release ${NEW_VERSION}

Changes:
"
if [[ -n "$FEATURES" ]]; then
    TAG_MSG+="
New Features:
$(echo "$FEATURES" | head -5)"
fi

if [[ -n "$FIXES" ]]; then
    TAG_MSG+="

Bug Fixes:
$(echo "$FIXES" | head -5)"
fi

TAG_MSG+="

Full changelog: https://github.com/andynicholson/MetaImGUI/blob/main/CHANGELOG.md"

# Create annotated tag
git tag -a "$NEW_VERSION" -m "$TAG_MSG"
echo -e "${GREEN}✓ Created tag ${NEW_VERSION}${NC}"

echo
echo -e "${BOLD}${GREEN}Release ${NEW_VERSION} prepared successfully!${NC}"
echo
echo -e "${BOLD}Next steps:${NC}"
echo "1. Review changes: git show"
echo "2. Push commit:    git push origin main"
echo "3. Push tag:       git push origin ${NEW_VERSION}"
echo
echo "Or push both:      git push origin main ${NEW_VERSION}"
echo
echo -e "${YELLOW}Note: Pushing the tag will trigger the release workflow${NC}"

