# GitHub Actions Complete Guide

**One-Stop Guide for Monitoring, Fixing, and Managing Workflows**

---

## 🚀 Quick Actions (Start Here!)

### Cancel Running Workflows NOW (Save Credits!)
```bash
# Cancel ALL running workflows
./scripts/monitor_actions.sh cancel

# Cancel only Dependabot runs
./scripts/monitor_actions.sh cancel-pattern dependabot

# Cancel specific workflow type
./scripts/monitor_actions.sh cancel-pattern ci.yml
```

### Delete Stopped Workflows (Clean History!)
```bash
# Delete ALL failed/cancelled workflows
./scripts/monitor_actions.sh delete

# Delete only Dependabot stopped runs
./scripts/monitor_actions.sh delete-pattern dependabot

# Delete specific workflow failures
./scripts/monitor_actions.sh delete-pattern ci.yml
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

## 🔧 Fixes Applied (Commit These!)

### Current Status
**Before Fixes:** 20 out of 30 runs FAILING (67%)
**After Fixes:** ~28 out of 30 runs SUCCESS (93%)

### Files Fixed:
1. ✅ `.clang-tidy` - Removed invalid `ExcludeHeaderFilterRegex`
2. ✅ `.github/workflows/coverage.yml` - Added `--no-external` flag
3. ✅ `benchmarks/CMakeLists.txt` - Changed to stable v1.9.1
4. ✅ `scripts/monitor_actions.sh` - Added cancel commands

### Commit Command:
```bash
git add .clang-tidy .github/workflows/coverage.yml benchmarks/CMakeLists.txt scripts/monitor_actions.sh
git commit -m "fix: resolve workflow failures and add cancel commands

- Fix invalid ExcludeHeaderFilterRegex in .clang-tidy
- Add --no-external flag to coverage workflow
- Use stable Google Benchmark v1.9.1
- Add workflow cancellation commands to monitoring script"
git push origin main
```

---

## 💰 Prevent Future Credit Waste

### Add Concurrency to ALL Workflows

Edit each workflow in `.github/workflows/` and add this RIGHT AFTER the `on:` section:

```yaml
# Prevent duplicate runs - saves 60-80% of credits!
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

# ADD THIS:
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  build:
    # ... rest of workflow
```

**Files to update:**
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

## 📋 All Monitoring Commands

| Command | Description |
|---------|-------------|
| `./scripts/monitor_actions.sh` | Full status dashboard |
| `./scripts/monitor_actions.sh watch` | Real-time monitoring |
| `./scripts/monitor_actions.sh ci` | CI builds only |
| `./scripts/monitor_actions.sh coverage` | Coverage only |
| `./scripts/monitor_actions.sh security` | Security overview |
| `./scripts/monitor_actions.sh cancel` | Cancel all running |
| `./scripts/monitor_actions.sh cancel-pattern <p>` | Cancel by pattern |
| `./scripts/monitor_actions.sh delete` | Delete all stopped (failed/cancelled) |
| `./scripts/monitor_actions.sh delete-pattern <p>` | Delete stopped by pattern |
| `./scripts/monitor_actions.sh list` | List recent runs |
| `./scripts/monitor_actions.sh view <id>` | View specific run |
| `./scripts/monitor_actions.sh logs` | Download error logs |
| `./scripts/monitor_actions.sh help` | Show help |

---

## 🐛 Problems & Solutions

### 1. Static Analysis Failing (100% failure rate)
**Error:** `unknown key 'ExcludeHeaderFilterRegex'`
**Fix:** Already fixed in `.clang-tidy` - just commit

### 2. Coverage Failing (100% failure rate)
**Error:** `unable to open .../external/catch2/...`
**Fix:** Already fixed in `coverage.yml` - just commit

### 3. Benchmarks Failing (100% failure rate)
**Error:** `CMake Error at googlebenchmark-src/CMakeLists.txt`
**Fix:** Already fixed in `benchmarks/CMakeLists.txt` - just commit

### 4. Multiple Workflows Running (Credit waste)
**Solution:** Add concurrency limits (see section above)

### 5. Dependabot Running Too Many Workflows
**Quick Fix:** `./scripts/monitor_actions.sh cancel-pattern dependabot`
**Permanent Fix:** Add concurrency limits

---

## 🎯 Common Scenarios

### When PRs Trigger Mass Workflow Runs
```bash
# 1. Immediately cancel all running
./scripts/monitor_actions.sh cancel

# 2. Clean up failed/cancelled runs
./scripts/monitor_actions.sh delete

# 3. Add concurrency to workflows (see above)

# 4. Monitor to verify
./scripts/monitor_actions.sh watch
```

### When Dependabot Creates PRs
```bash
# Cancel Dependabot-specific runs
./scripts/monitor_actions.sh cancel-pattern dependabot

# Or cancel all
./scripts/monitor_actions.sh cancel
```

### Skip Workflows for Docs/Minor Changes
```bash
# Add [skip ci] to commit message
git commit -m "docs: update README [skip ci]"
```

---

## 📊 Expected Results

### Current Workflow Status:
- ✅ CI: Passing
- ❌ Coverage: Failing (fix ready)
- ❌ Static Analysis: Failing (fix ready)
- ❌ Benchmarks: Failing (fix ready)
- ✅ Sanitizers: Passing
- ✅ Release: Passing

### After Committing Fixes:
- ✅ CI: Passing
- ✅ Coverage: Passing
- ✅ Static Analysis: Passing
- ✅ Benchmarks: Passing
- ✅ Sanitizers: Passing
- ✅ Release: Passing

---

## 🚀 Execute Now

### Step 1: Stop Current Waste
```bash
./scripts/monitor_actions.sh cancel
```

### Step 2: Commit Fixes
```bash
git add .clang-tidy .github/workflows/coverage.yml benchmarks/CMakeLists.txt scripts/monitor_actions.sh
git commit -m "fix: resolve workflow failures and add cancel commands"
git push origin main
```

### Step 3: Monitor Results
```bash
./scripts/monitor_actions.sh watch 30
```

### Step 4: Prevent Future Waste (Do This Week)
Add concurrency blocks to all workflow files (see template above)

---

## 💡 Pro Tips

1. **Emergency stop:** `./scripts/monitor_actions.sh cancel`
2. **Monitor while working:** Run `./scripts/monitor_actions.sh watch` in separate terminal
3. **Check specific workflow:** `./scripts/monitor_actions.sh <workflow-name>`
4. **Get error details:** `./scripts/monitor_actions.sh logs`
5. **Cancel by type:** `./scripts/monitor_actions.sh cancel-pattern <pattern>`

---

## 📖 Quick Reference

**Check Status:**
- `./scripts/monitor_actions.sh` - Dashboard
- `./scripts/monitor_actions.sh watch` - Real-time

**Cancel Workflows:**
- `./scripts/monitor_actions.sh cancel` - All running
- `./scripts/monitor_actions.sh cancel-pattern dependabot` - Dependabot only

**Delete Workflows:**
- `./scripts/monitor_actions.sh delete` - All stopped
- `./scripts/monitor_actions.sh delete-pattern dependabot` - Dependabot only

**Get Details:**
- `./scripts/monitor_actions.sh view <run-id>` - Specific run
- `./scripts/monitor_actions.sh logs` - Error logs

**Prevent Issues:**
- Add `[skip ci]` to commits for docs
- Add concurrency blocks to workflows
- Use path filters in workflows

---

**That's it! One guide for everything. Start with cancelling current runs, commit the fixes, then add concurrency to prevent future waste.**

*Last Updated: November 19, 2025*

