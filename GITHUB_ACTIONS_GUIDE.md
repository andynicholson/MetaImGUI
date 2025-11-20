# GitHub Actions Guide

**Comprehensive Guide for Monitoring, Fixing, and Managing Workflows**

---

## Quick Reference

### Cancel Running Workflows
```bash
# Cancel all running workflows
./scripts/monitor_actions.sh cancel

# Cancel only Dependabot runs
./scripts/monitor_actions.sh cancel-pattern dependabot

# Cancel specific workflow type
./scripts/monitor_actions.sh cancel-pattern ci.yml
```

### Delete Workflows
```bash
# Delete all failed/cancelled workflows
./scripts/monitor_actions.sh delete

# Delete only Dependabot stopped runs
./scripts/monitor_actions.sh delete-pattern dependabot

# Delete specific workflow failures
./scripts/monitor_actions.sh delete-pattern ci.yml

# Delete all workflows except those from latest commit
./scripts/monitor_actions.sh delete-old
```

### Check Status
```bash
# Full dashboard
./scripts/monitor_actions.sh

# Watch in real-time (refresh every 30s)
./scripts/monitor_actions.sh watch

# Specific workflows
./scripts/monitor_actions.sh ci
./scripts/monitor_actions.sh coverage
./scripts/monitor_actions.sh security
```

---

## Workflow Configuration

### Concurrency Control

To prevent duplicate workflow runs and reduce CI credit consumption, add concurrency control to workflows in `.github/workflows/`. Place this configuration immediately after the `on:` section:

```yaml
# Prevent duplicate runs
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

**Example:**
```yaml
name: CI

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: cmake --build build
      - name: Test
        run: ctest --test-dir build
```

**Applicable workflow files:**
- `ci.yml`
- `coverage.yml`
- `static-analysis.yml`
- `sanitizers.yml`
- `benchmarks.yml`
- `codeql.yml`
- `dependency-review.yml`
- `release.yml`
- `docs.yml`

---

## Monitoring Commands

### Status and Monitoring

| Command | Description |
|---------|-------------|
| `./scripts/monitor_actions.sh` | Full status dashboard (default) |
| `./scripts/monitor_actions.sh status` | Full status dashboard |
| `./scripts/monitor_actions.sh watch [seconds]` | Real-time monitoring (default: 30s refresh) |
| `./scripts/monitor_actions.sh list [workflow]` | List recent runs (all or specific workflow) |
| `./scripts/monitor_actions.sh view <run_id>` | View detailed information for a specific run |

### Workflow-Specific Views

| Command | Description |
|---------|-------------|
| `./scripts/monitor_actions.sh ci` | CI builds only |
| `./scripts/monitor_actions.sh coverage` | Coverage status only |
| `./scripts/monitor_actions.sh codeql` | CodeQL security analysis only |
| `./scripts/monitor_actions.sh release` | Release builds only |
| `./scripts/monitor_actions.sh dependabot` | Dependabot and dependency review |
| `./scripts/monitor_actions.sh static-analysis` | Static analysis and sanitizers |
| `./scripts/monitor_actions.sh benchmarks` | Benchmark results |
| `./scripts/monitor_actions.sh security` | Security overview (CodeQL + Dependabot) |

### Workflow Management

| Command | Description |
|---------|-------------|
| `./scripts/monitor_actions.sh cancel` | Cancel all running/queued workflows (interactive) |
| `./scripts/monitor_actions.sh cancel-pattern <pattern>` | Cancel workflows matching pattern |
| `./scripts/monitor_actions.sh delete` | Delete all stopped (failed/cancelled) workflows |
| `./scripts/monitor_actions.sh delete-pattern <pattern>` | Delete stopped workflows matching pattern |
| `./scripts/monitor_actions.sh delete-old` | Delete all workflows except those from latest commit |

### Log Analysis

| Command | Description |
|---------|-------------|
| `./scripts/monitor_actions.sh logs [workflow]` | Download and analyze logs (all failed or specific workflow) |

### Examples

```bash
# Watch workflows with 10-second refresh
./scripts/monitor_actions.sh watch 10

# List only CI runs
./scripts/monitor_actions.sh list ci.yml

# Cancel all Dependabot workflows
./scripts/monitor_actions.sh cancel-pattern dependabot

# Delete workflows from older commits (keep only latest)
./scripts/monitor_actions.sh delete-old

# Download logs for failed coverage runs
./scripts/monitor_actions.sh logs coverage.yml
```

---

## Troubleshooting

### Static Analysis Failures
**Error:** `unknown key 'ExcludeHeaderFilterRegex'`

**Solution:** The `ExcludeHeaderFilterRegex` key is not valid in `.clang-tidy` configuration. Remove this key from the configuration file.

### Coverage Failures
**Error:** `unable to open .../external/catch2/...`

**Solution:** Add the `--no-external` flag to the coverage workflow in `.github/workflows/coverage.yml` to exclude external dependencies from coverage analysis.

### Benchmark Failures
**Error:** `CMake Error at googlebenchmark-src/CMakeLists.txt`

**Solution:** Use a stable version of Google Benchmark (e.g., v1.9.1) in `benchmarks/CMakeLists.txt` instead of the master branch.

### Multiple Concurrent Workflows
**Issue:** Multiple workflow instances run simultaneously, consuming excessive CI credits.

**Solution:** Add concurrency controls to workflow files (see Concurrency Control section above).

### Excessive Dependabot Workflows
**Issue:** Dependabot pull requests trigger numerous workflow runs.

**Solution:**
- Immediate: `./scripts/monitor_actions.sh cancel-pattern dependabot`
- Long-term: Add concurrency controls to workflow files

---

## Common Workflows

### Managing Mass Workflow Runs
When multiple workflows run simultaneously (e.g., from pull requests):

```bash
# 1. Cancel all running workflows
./scripts/monitor_actions.sh cancel

# 2. Clean up failed/cancelled runs
./scripts/monitor_actions.sh delete

# 3. Add concurrency controls to prevent recurrence

# 4. Monitor workflow status
./scripts/monitor_actions.sh watch
```

### Managing Dependabot Workflows
When Dependabot creates pull requests that trigger multiple workflows:

```bash
# Cancel Dependabot-specific runs
./scripts/monitor_actions.sh cancel-pattern dependabot

# Delete stopped Dependabot runs
./scripts/monitor_actions.sh delete-pattern dependabot

# View Dependabot-specific status
./scripts/monitor_actions.sh dependabot
```

### Cleaning Up Old Workflow History
To maintain a clean workflow history, delete workflows from older commits while preserving runs from the latest commit:

```bash
# Delete all workflows except those from the latest commit
./scripts/monitor_actions.sh delete-old
```

This command is useful for:
- Cleaning up after rebases or force pushes
- Removing outdated workflow runs from old branches
- Maintaining a clean workflow history focused on current work

### Skipping Workflows
To skip workflow execution for documentation or minor changes, include `[skip ci]` in the commit message:

```bash
git commit -m "docs: update README [skip ci]"
```

---

## Workflow Overview

The project includes the following GitHub Actions workflows:

- **CI** (`ci.yml`): Builds and tests the codebase
- **Coverage** (`coverage.yml`): Generates code coverage reports with percentage tracking
- **Static Analysis** (`static-analysis.yml`): Runs clang-tidy checks
- **Benchmarks** (`benchmarks.yml`): Executes performance benchmarks
- **Sanitizers** (`sanitizers.yml`): Runs address and undefined behavior sanitizers
- **CodeQL** (`codeql.yml`): Performs security analysis with alert tracking
- **Dependency Review** (`dependency-review.yml`): Reviews dependency changes
- **Release** (`release.yml`): Builds and publishes releases
- **Docs** (`docs.yml`): Generates and deploys documentation

The monitoring script provides specialized views for each workflow type, allowing focused investigation of specific build, test, security, or deployment issues.

---

## Best Practices

### Monitoring

1. Use `./scripts/monitor_actions.sh` or `./scripts/monitor_actions.sh status` for a comprehensive dashboard
2. Run `./scripts/monitor_actions.sh watch` in a separate terminal for continuous monitoring
3. Use workflow-specific commands (`ci`, `coverage`, `security`, etc.) to focus on particular areas
4. Review error details with `./scripts/monitor_actions.sh logs [workflow]`
5. View detailed run information with `./scripts/monitor_actions.sh view <run_id>`

### Workflow Management

**View workflow status:**
- `./scripts/monitor_actions.sh` - Complete dashboard
- `./scripts/monitor_actions.sh watch [seconds]` - Real-time updates with custom refresh interval
- `./scripts/monitor_actions.sh <workflow>` - Specific workflow (ci, coverage, codeql, release, etc.)

**Cancel running workflows:**
- `./scripts/monitor_actions.sh cancel` - All running/queued workflows (interactive confirmation)
- `./scripts/monitor_actions.sh cancel-pattern <pattern>` - Workflows matching a pattern

**Delete workflow history:**
- `./scripts/monitor_actions.sh delete` - All stopped (failed/cancelled) workflows
- `./scripts/monitor_actions.sh delete-pattern <pattern>` - Stopped workflows matching a pattern
- `./scripts/monitor_actions.sh delete-old` - All workflows except those from the latest commit

**Analyze workflow runs:**
- `./scripts/monitor_actions.sh view <run_id>` - Detailed information for a specific run
- `./scripts/monitor_actions.sh logs [workflow]` - Download and extract errors from logs
- `./scripts/monitor_actions.sh list [workflow]` - List recent runs

### Resource Optimization

- Add `[skip ci]` to commit messages for documentation changes
- Configure concurrency controls in workflow files to prevent duplicate runs
- Use path filters to limit workflow triggers to relevant file changes
- Regularly clean up old workflow runs with `delete-old` after rebases or major refactors
- Use pattern-based cancellation/deletion for bulk management of specific workflow types

---

## Requirements and Setup

### Dependencies

The monitoring script requires the following tools:

- **gh** (GitHub CLI) - Required for interacting with GitHub Actions
- **jq** - Required for JSON processing
- **git** - Required for repository detection
- Standard Unix tools (grep, sed, awk, xargs)

### Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install gh jq
```

**macOS:**
```bash
brew install gh jq
```

### Authentication

After installing the GitHub CLI, authenticate with:

```bash
gh auth login
```

### First Run

On first run, the script automatically detects the repository from the git remote configuration and stores this information in `~/.metaimgui_monitor_config` for subsequent runs.

### Log Files

The script stores log files in the following locations:

- Workflow logs: `./workflow_logs/`
- Error extracts: `./workflow_logs/run_<id>_errors.txt`
- Configuration: `~/.metaimgui_monitor_config`
- Cache directory: `~/.cache/metaimgui_monitor`

---

## Script Features

- **Comprehensive monitoring**: Tracks all workflow types (CI, coverage, security, etc.)
- **Real-time updates**: Watch mode with configurable refresh intervals
- **Error extraction**: Automatic extraction and highlighting of errors from failed runs
- **Coverage tracking**: Extracts and displays code coverage percentages
- **Security alerts**: Displays CodeQL and Dependabot alerts with severity levels
- **Interactive management**: Confirms before canceling or deleting workflows
- **Batch operations**: Parallel deletion of workflows for improved performance
- **Pattern matching**: Filter workflows by name or pattern for targeted operations
- **Commit-aware cleanup**: Delete old workflows while preserving runs from latest commit

---

*Last Updated: November 20, 2025*

