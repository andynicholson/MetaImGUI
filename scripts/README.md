# Scripts

## GitHub Actions Monitoring

For complete documentation, see: **[`../GITHUB_ACTIONS_GUIDE.md`](../GITHUB_ACTIONS_GUIDE.md)**

```bash
./scripts/monitor_actions.sh              # Show status
./scripts/monitor_actions.sh delete       # Delete failed runs
./scripts/monitor_actions.sh cancel       # Cancel running workflows
```

## Release Preparation

```bash
./scripts/prepare_release.sh              # Interactive release preparation
```

Automatically:
- Suggests next version (patch/minor/major)
- Generates changelog from commits
- Updates CHANGELOG.md, CMakeLists.txt, README.md
- Creates release commit and annotated tag

## Requirements

```bash
sudo apt-get install gh jq
gh auth login
```

---

**Full documentation, examples, and troubleshooting:** [`../GITHUB_ACTIONS_GUIDE.md`](../GITHUB_ACTIONS_GUIDE.md)
