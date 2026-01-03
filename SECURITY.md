# Security Policy

## Supported Versions

We release patches for security vulnerabilities in the following versions:

| Version | Supported          |
| ------- | ------------------ |
| Latest  | :white_check_mark: |
| < Latest| :x:                |

**Note**: As MetaImGUI is a template project, we recommend always using the latest version. Security updates will be applied to the main branch and included in the next release.

## Automated Security Measures

MetaImGUI employs multiple automated security scanning tools:

- **CodeQL Analysis**: Continuous security analysis for C++ code vulnerabilities
- **Dependabot**: Automated dependency updates and vulnerability alerts
- **Dependency Review**: PR-level dependency vulnerability scanning
- **Memory Sanitizers**: ASAN, TSAN, UBSAN, MSAN for runtime error detection

These automated checks run on every pull request and push to main/develop branches.

## Reporting a Vulnerability

We take the security of MetaImGUI seriously. If you believe you have found a security vulnerability, please report it to us as described below.

### Where to Report

**Please do NOT report security vulnerabilities through public GitHub issues.**

Instead, please report them via email to:

**Security Contact**: intothemist@gmail.com

Please include the following information:

- Type of vulnerability
- Full description of the vulnerability
- Steps to reproduce the issue
- Potential impact
- Suggested fix (if any)
- Your contact information for follow-up questions

### What to Expect

When you report a vulnerability, you can expect:

1. **Acknowledgment**: We will acknowledge receipt of your vulnerability report within 48 hours.

2. **Assessment**: We will investigate and validate the vulnerability within 7 days.

3. **Updates**: We will keep you informed about the progress of resolving the issue.

4. **Fix Timeline**:
   - **Critical vulnerabilities**: Fixed within 7 days
   - **High severity**: Fixed within 30 days
   - **Medium/Low severity**: Fixed in the next scheduled release

5. **Credit**: If you wish, we will credit you in the security advisory and release notes.

### Disclosure Policy

- Please give us reasonable time to address the vulnerability before any public disclosure.
- We will coordinate with you on the disclosure timeline.
- Once a fix is available, we will publish a security advisory on GitHub.

## Security Considerations for Template Users

When using MetaImGUI as a template for your own applications, consider these security best practices:

### 1. Dependencies

- **Keep dependencies up to date**: Regularly update ImGui, GLFW, and other dependencies
- **Review third-party code**: Audit any additional dependencies you add
- **Monitor CVE databases**: Watch for vulnerabilities in dependencies

### 2. Network Security

The UpdateChecker makes HTTP(S) requests to GitHub:
- Requests are made over HTTPS to api.github.com
- No sensitive data is transmitted
- Consider adding certificate pinning for production applications
- Review and audit network code before deployment

### 3. Input Validation

- **Validate user input**: Always sanitize and validate data from UI controls
- **File paths**: Sanitize file paths to prevent directory traversal
- **Configuration files**: Validate JSON configuration before parsing

### 4. Build Security

- **Compiler warnings**: Treat warnings as errors (`-Werror` in CMakeLists.txt)
- **Static analysis**: Run clang-tidy regularly
- **Address sanitizers**: Use AddressSanitizer and UndefinedBehaviorSanitizer during development
- **Enable security features**: Use compiler hardening flags in production:
  - `-D_FORTIFY_SOURCE=2`
  - `-fstack-protector-strong`
  - `-fPIE -pie` (Position Independent Executable)
  - `-Wl,-z,relro,-z,now` (Full RELRO)

### 5. Platform-Specific Security

**Windows**:
- Enable DEP (Data Execution Prevention)
- Enable ASLR (Address Space Layout Randomization)
- Code signing for executables

**Linux**:
- Use AppArmor or SELinux profiles
- Minimize capabilities and permissions
- Secure file permissions

**macOS**:
- Code signing and notarization
- Hardened runtime
- Use entitlements appropriately

### 6. Credentials and Secrets

- **Never hardcode secrets**: Don't commit API keys, passwords, or tokens
- **Use environment variables**: For configuration requiring secrets
- **Secure storage**: Use platform keychain/credential managers
- **GitHub Actions secrets**: For CI/CD credentials

### 7. Update Mechanism Security

The built-in UpdateChecker:
- Only notifies users of updates; doesn't auto-download
- Verifies HTTPS connection to GitHub
- Users should verify download integrity using provided SHA256 checksums
- Consider implementing GPG signature verification for releases

## Known Security Considerations

### Update Checker

The UpdateChecker component fetches release information from GitHub's API:

- **Communication**: Uses HTTPS (TLS encrypted)
- **Data parsed**: JSON response from GitHub API
- **No code execution**: Only displays update information
- **User action required**: Downloads happen through user's browser, not automatically

### Configuration Files

Settings may be stored in JSON files:

- **Location**: `~/.config/MetaImGUI/` (Linux), `AppData` (Windows)
- **Content**: Window settings, theme preferences (no sensitive data by default)
- **Recommendation**: Validate and sanitize before parsing
- **Consideration**: File permissions should be user-readable only

### Third-Party Dependencies

Current dependencies and their security status:

- **ImGui**: Regularly updated, actively maintained
- **GLFW**: Mature library, good security track record
- **nlohmann/json**: Well-tested, widely used
- **libcurl**: Keep updated, has security team
- **Catch2**: Testing framework, not in production binaries

## Security Checklist for Forked Projects

When creating an application from this template:

- [ ] Review all dependencies for known vulnerabilities
- [ ] Update to latest stable versions of all dependencies
- [ ] Enable compiler security flags in CMakeLists.txt
- [ ] Implement input validation for all user inputs
- [ ] Add authentication/authorization if handling sensitive data
- [ ] Use encrypted connections for network communication
- [ ] Implement secure credential storage
- [ ] Regular security audits and penetration testing
- [ ] Set up automated dependency scanning (Dependabot, Snyk)
- [ ] Code sign your releases
- [ ] Provide SHA256 checksums for downloads
- [ ] Document security considerations in your README

## Responsible Disclosure Hall of Fame

We appreciate security researchers who responsibly disclose vulnerabilities. Contributors who help improve security will be recognized here:

*(No vulnerabilities reported yet)*

## Security Updates

Security updates will be announced:

1. **GitHub Security Advisories**: https://github.com/andynicholson/MetaImGUI/security/advisories
2. **Release Notes**: Tagged releases on GitHub
3. **CHANGELOG.md**: Documented in changelog

Subscribe to repository notifications to receive security updates.

## Questions?

If you have questions about this security policy, please contact: intothemist@gmail.com

---

Last updated: 2025-12-03

