# Security Checklist for Releases

This checklist must be completed before every release to ensure security standards are met.

## Pre-Release Security Verification

### Automated Scans

- [ ] **CodeQL Analysis**: All CodeQL scans pass with no critical/high vulnerabilities
- [ ] **Dependabot**: All dependency updates reviewed and applied
- [ ] **Dependency Review**: No blocked dependencies in recent PRs
- [ ] **Memory Sanitizers**: All sanitizer builds (ASAN, TSAN, UBSAN) pass
- [ ] **Static Analysis**: clang-tidy and cppcheck pass with no critical warnings

### Dependency Audit

- [ ] **External Dependencies**: pinned to commit SHAs in
      `setup_dependencies.sh` and `.github/workflows/ci.yml`. To bump:
  - [ ] Resolve the new tag → SHA, update both files in lockstep
  - [ ] ImGui (currently v1.92.4)
  - [ ] nlohmann/json (currently v3.11.3)
  - [ ] ImPlot (currently v0.17)
  - [ ] Catch2 (currently v3.4.0)
  - [ ] GLFW comes from system packages (apt / Homebrew / vcpkg) — confirm
        a supported version on each runner image
- [ ] **License Compliance**: All dependencies have compatible licenses
- [ ] **CVE Check**: No known CVEs in dependency versions
- [ ] **Pin Discipline**: Every third-party clone uses `--detach <sha>`,
      never a bare tag/branch (verifiable via `grep -r "branch v" .github/`
      and `setup_dependencies.sh` — should return zero hits)

### Code Review

- [ ] **Security-Sensitive Code**: All authentication, file I/O, network code reviewed
- [ ] **Input Validation**: All user inputs validated and sanitized
- [ ] **Buffer Safety**: No buffer overflows possible
- [ ] **Memory Management**: No memory leaks detected
- [ ] **Thread Safety**: No data races in multi-threaded code

### Build & Deployment

> The hardening flags below are **not** wired into CMakeLists.txt yet — they
> are aspirational items, kept in this list so we don't lose track. Treat
> them as "must add before claiming hardened builds", not "tick off and ship."

- [ ] **Compiler Flags**: Release builds should use security-hardening flags
  - [ ] `-D_FORTIFY_SOURCE=2` (NOT YET WIRED — add to CMake Release config)
  - [ ] `-fstack-protector-strong` (NOT YET WIRED)
  - [ ] `-fPIE -pie` for PIE executables (NOT YET WIRED)
  - [ ] `-Wl,-z,relro,-z,now` for full RELRO (NOT YET WIRED)
- [ ] **Binary Hardening**: verify on the produced artifact
  - [ ] ASLR (PIE) — `checksec --file=build/MetaImGUI`
  - [ ] NX / DEP — usually default; verify with checksec
  - [ ] RELRO — verify with checksec
- [ ] **Signing**: Release artifacts are signed (if applicable)

### Testing

- [ ] **Existing Tests**: `ctest` passes (Catch2 suite, with display tests
      under `xvfb-run` on Linux CI)
- [ ] **Sanitizer Builds**: `.github/workflows/sanitizers.yml` is green
      (ASan / UBSan / TSan)
- [ ] **Fuzzing**: NOT IMPLEMENTED — no fuzz harnesses exist yet
- [ ] **Stress Tests**: NOT IMPLEMENTED — only the ConfigManager threading
      test exercises concurrent access

### Documentation

- [ ] **SECURITY.md**: Updated with current version information
- [ ] **Changelog**: Security fixes documented
- [ ] **Release Notes**: Security improvements highlighted
- [ ] **Known Issues**: Any known security limitations documented

### Incident Response

- [ ] **Contact Information**: Security contact email is current
- [ ] **Response Plan**: Team knows how to respond to security reports
- [ ] **Disclosure Policy**: Responsible disclosure process documented

## Release Sign-Off

**Release Version**: ________________

**Date**: ________________

**Verified By**: ________________

**Notes**:
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________

## Post-Release

- [ ] **GitHub Release**: Security tab reviewed for new alerts
- [ ] **Monitoring**: Watch for security reports after release
- [ ] **Feedback**: Monitor issue tracker for security-related reports

---

## Critical Severity Levels

- **Critical**: Remote code execution, privilege escalation, data breach
- **High**: Authentication bypass, XSS, SQL injection, memory corruption
- **Medium**: Denial of service, information disclosure
- **Low**: Configuration issues, deprecated features

**If any CRITICAL or HIGH severity issues are found, STOP the release and address them immediately.**

