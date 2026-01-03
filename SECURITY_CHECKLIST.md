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

- [ ] **External Dependencies**: All external dependencies are up-to-date
  - [ ] ImGui version checked and updated if needed
  - [ ] nlohmann/json version checked and updated if needed
  - [ ] GLFW version compatible and secure
- [ ] **License Compliance**: All dependencies have compatible licenses
- [ ] **CVE Check**: No known CVEs in dependency versions
- [ ] **Submodule Verification**: All git submodules point to trusted commits

### Code Review

- [ ] **Security-Sensitive Code**: All authentication, file I/O, network code reviewed
- [ ] **Input Validation**: All user inputs validated and sanitized
- [ ] **Buffer Safety**: No buffer overflows possible
- [ ] **Memory Management**: No memory leaks detected
- [ ] **Thread Safety**: No data races in multi-threaded code

### Build & Deployment

- [ ] **Compiler Flags**: Release builds use security-hardening flags
  - [ ] `-D_FORTIFY_SOURCE=2` enabled
  - [ ] `-fstack-protector-strong` enabled
  - [ ] `-fPIE -pie` for position-independent executables
- [ ] **Binary Hardening**: Release binaries are hardened
  - [ ] ASLR enabled
  - [ ] NX bit set
  - [ ] RELRO enabled
- [ ] **Signing**: Release artifacts are signed (if applicable)

### Testing

- [ ] **Security Tests**: All security-focused tests pass
- [ ] **Fuzzing**: Fuzz tests run without crashes (if implemented)
- [ ] **Integration Tests**: All integration tests pass
- [ ] **Stress Tests**: Long-running stress tests complete successfully

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

