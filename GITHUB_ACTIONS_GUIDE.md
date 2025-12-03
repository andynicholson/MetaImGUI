# GitHub Actions Guide

Monitoring and managing workflows.

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

Prevents duplicate workflow runs. Add to `.github/workflows/` files after the `on:` section:

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

Applies to: `ci.yml`, `coverage.yml`, `static-analysis.yml`, `sanitizers.yml`, `benchmarks.yml`, `codeql.yml`, `dependency-review.yml`, `release.yml`, `docs.yml`

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
Remove `ExcludeHeaderFilterRegex` from `.clang-tidy` (invalid key).

### Coverage Failures
Add `--no-external` flag to `coverage.yml` to exclude external dependencies.

### Benchmark Failures
Use stable Google Benchmark version (v1.9.1) in `benchmarks/CMakeLists.txt`.

### Multiple Concurrent Workflows
Add concurrency controls to workflow files.

### Excessive Dependabot Workflows
Run `./scripts/monitor_actions.sh cancel-pattern dependabot` or add concurrency controls.

---

## Common Workflows

### Managing Mass Workflow Runs
```bash
./scripts/monitor_actions.sh cancel
./scripts/monitor_actions.sh delete
./scripts/monitor_actions.sh watch
```

### Managing Dependabot Workflows
```bash
./scripts/monitor_actions.sh cancel-pattern dependabot
./scripts/monitor_actions.sh delete-pattern dependabot
./scripts/monitor_actions.sh dependabot
```

### Cleaning Up Old Workflow History
```bash
./scripts/monitor_actions.sh delete-old
```

Useful after rebases or for removing outdated runs.

### Skipping Workflows
```bash
git commit -m "docs: update README [skip ci]"
```

---

## Workflow Overview

GitHub Actions workflows:

- `ci.yml`: Builds and tests
- `coverage.yml`: Code coverage reports
- `static-analysis.yml`: clang-tidy, cppcheck
- `benchmarks.yml`: Performance benchmarks
- `sanitizers.yml`: ASan, UBSan, TSan
- `codeql.yml`: Security analysis
- `dependency-review.yml`: Dependency scanning
- `release.yml`: Release builds
- `docs.yml`: Documentation generation

---

## Best Practices

### Monitoring
- `./scripts/monitor_actions.sh` - Dashboard
- `./scripts/monitor_actions.sh watch` - Continuous monitoring
- `./scripts/monitor_actions.sh <workflow>` - Specific workflow
- `./scripts/monitor_actions.sh logs [workflow]` - Error details

### Workflow Management
- `./scripts/monitor_actions.sh cancel` - Cancel running
- `./scripts/monitor_actions.sh delete` - Delete stopped
- `./scripts/monitor_actions.sh delete-old` - Clean old runs
- `./scripts/monitor_actions.sh view <run_id>` - Run details

### Resource Optimization
- Use `[skip ci]` for doc changes
- Add concurrency controls
- Clean old runs with `delete-old`
- Use pattern-based operations for bulk management

---

## Requirements

Monitoring script requires:
- GitHub CLI (`gh`)
- `jq`
- `git`
- Standard Unix tools

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
```bash
gh auth login
```

### Log Files
- Workflow logs: `./workflow_logs/`
- Errors: `./workflow_logs/run_<id>_errors.txt`
- Config: `~/.metaimgui_monitor_config`
- Cache: `~/.cache/metaimgui_monitor`

---

## Script Features

- Tracks all workflow types
- Real-time watch mode
- Error extraction from logs
- Coverage tracking
- Security alerts (CodeQL, Dependabot)
- Interactive confirmations
- Batch operations
- Pattern matching
- Commit-aware cleanup

---

*Last Updated: December 3, 2025*

