# Security Practices for Development

This guide outlines security best practices for developers contributing to MetaImGUI.

## Table of Contents

1. [Secure Coding Principles](#secure-coding-principles)
2. [Common Vulnerabilities](#common-vulnerabilities)
3. [Input Validation](#input-validation)
4. [Memory Safety](#memory-safety)
5. [Dependency Management](#dependency-management)
6. [Testing for Security](#testing-for-security)
7. [Code Review Guidelines](#code-review-guidelines)

---

## Secure Coding Principles

### Defense in Depth

Apply multiple layers of security controls:
- Validate inputs at all boundaries
- Use safe APIs and libraries
- Handle errors securely
- Apply principle of least privilege
- Assume external data is malicious

### Fail Securely

When errors occur, fail in a secure state:
```cpp
// Good: Secure failure
bool LoadConfig(const std::string& path) {
    if (!ValidatePath(path)) {
        LOG_ERROR("Invalid config path");
        return false;  // Fail securely
    }
    // ... load config
}

// Bad: Continuing after validation failure
bool LoadConfig(const std::string& path) {
    ValidatePath(path);  // Ignoring return value
    // ... load config anyway
}
```

### Keep Security Simple

Complex security mechanisms are error-prone:
- Use well-tested libraries (OpenSSL, not custom crypto)
- Prefer standard algorithms
- Keep security-critical code minimal and reviewable

---

## Common Vulnerabilities

### Buffer Overflows

**Risk**: Memory corruption, code execution

**Prevention**:
```cpp
// Bad: Fixed-size buffer
char buffer[256];
strcpy(buffer, user_input);  // Overflow risk!

// Good: Use std::string
std::string buffer = user_input;

// Good: Use safe functions with bounds checking
char buffer[256];
strncpy(buffer, user_input, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

### Use-After-Free

**Risk**: Memory corruption, crashes

**Prevention**:
```cpp
// Bad: Using raw pointers
auto* obj = new Object();
delete obj;
obj->Method();  // Use-after-free!

// Good: Use smart pointers
auto obj = std::make_unique<Object>();
// Automatic cleanup, can't use after free
```

### Integer Overflows

**Risk**: Buffer overflows, logic errors

**Prevention**:
```cpp
// Bad: Unchecked arithmetic
size_t size = user_count * sizeof(Item);
auto* items = new Item[size];  // Overflow risk!

// Good: Check for overflow
if (user_count > SIZE_MAX / sizeof(Item)) {
    LOG_ERROR("Size overflow detected");
    return false;
}
size_t size = user_count * sizeof(Item);
```

### Path Traversal

**Risk**: Unauthorized file access

**Prevention**:
```cpp
// Bad: Using user input directly
std::string path = config_dir + "/" + user_filename;
std::ifstream file(path);  // Can access "../../../etc/passwd"!

// Good: Validate and canonicalize paths
std::filesystem::path safe_path = config_dir / user_filename;
safe_path = std::filesystem::canonical(safe_path);
if (!safe_path.string().starts_with(config_dir)) {
    LOG_ERROR("Path traversal attempt detected");
    return false;
}
```

---

## Input Validation

### Validate All External Input

**Sources requiring validation**:
- User-provided data (files, arguments, stdin)
- Configuration files
- Network data
- Environment variables

**Validation strategy**:
```cpp
bool ValidateConfigValue(const std::string& value) {
    // 1. Check type
    if (!IsValidFormat(value)) return false;

    // 2. Check range
    if (value.length() > MAX_CONFIG_VALUE_LEN) return false;

    // 3. Check content (whitelist preferred)
    if (!std::regex_match(value, ALLOWED_PATTERN)) return false;

    return true;
}
```

### Sanitize File Paths

```cpp
std::string SanitizeFilename(const std::string& filename) {
    // Remove directory separators
    std::string safe = filename;
    safe.erase(std::remove(safe.begin(), safe.end(), '/'), safe.end());
    safe.erase(std::remove(safe.begin(), safe.end(), '\\'), safe.end());

    // Limit length
    if (safe.length() > MAX_FILENAME_LEN) {
        safe = safe.substr(0, MAX_FILENAME_LEN);
    }

    // Reject dangerous names
    if (safe == "." || safe == ".." || safe.empty()) {
        throw std::invalid_argument("Invalid filename");
    }

    return safe;
}
```

---

## Memory Safety

### Use RAII

Resource Acquisition Is Initialization:
```cpp
// Good: Automatic resource management
class ConfigManager {
    std::unique_ptr<Config> m_config;
    std::ifstream m_file;

public:
    ConfigManager(const std::string& path)
        : m_config(std::make_unique<Config>()),
          m_file(path) {
        // Resources automatically cleaned up on exception
    }
    // Destructor automatically cleans up
};
```

### Avoid Manual Memory Management

```cpp
// Bad: Manual memory management
void ProcessData() {
    auto* data = new char[1024];
    Process(data);  // Might throw!
    delete[] data;  // Leak if exception occurs
}

// Good: Use containers
void ProcessData() {
    std::vector<char> data(1024);
    Process(data.data());  // Automatic cleanup
}

// Good: Use unique_ptr for arrays
void ProcessData() {
    auto data = std::make_unique<char[]>(1024);
    Process(data.get());  // Automatic cleanup
}
```

### Initialize All Variables

```cpp
// Bad: Uninitialized memory
int count;
if (condition) {
    count = 10;
}
// count might be uninitialized!

// Good: Always initialize
int count = 0;
if (condition) {
    count = 10;
}
```

---

## Dependency Management

### Keep Dependencies Updated

```bash
# Check for updates regularly
git submodule update --remote

# Review Dependabot PRs promptly
# Check GitHub Security advisories
```

### Verify Dependency Integrity

```bash
# For submodules: verify commit hashes
git submodule status

# For downloaded dependencies: verify checksums
sha256sum library.tar.gz
```

### Minimize Dependencies

- Only include dependencies that are necessary
- Prefer header-only libraries when practical
- Review dependency code for quality

---

## Testing for Security

### Memory Sanitizers

Run tests with sanitizers regularly:

```bash
# Address Sanitizer (memory errors)
cmake -B build -DENABLE_ASAN=ON
cmake --build build && cd build && ctest

# Thread Sanitizer (data races)
cmake -B build -DENABLE_TSAN=ON
cmake --build build && cd build && ctest

# Undefined Behavior Sanitizer
cmake -B build -DENABLE_UBSAN=ON
cmake --build build && cd build && ctest
```

### Fuzz Testing

Test with invalid/random inputs:

```cpp
// Example fuzz test
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);
    try {
        ConfigManager config;
        config.LoadFromString(input);  // Should not crash
    } catch (...) {
        // Exceptions are OK, crashes are not
    }
    return 0;
}
```

### Security-Focused Test Cases

Write tests for security scenarios:

```cpp
TEST_CASE("Path traversal is prevented") {
    ConfigManager config;
    REQUIRE_THROWS(config.Load("../../../etc/passwd"));
    REQUIRE_THROWS(config.Load("..\\..\\..\\windows\\system32\\config"));
}

TEST_CASE("Buffer overflow is prevented") {
    std::string huge_input(1024 * 1024 * 10, 'A');  // 10MB
    REQUIRE_NOTHROW(ProcessInput(huge_input));
}
```

---

## Code Review Guidelines

### Security Review Checklist

When reviewing code, check for:

- [ ] **Input Validation**: All external inputs validated
- [ ] **Buffer Safety**: No fixed-size buffers with user data
- [ ] **Integer Overflow**: Arithmetic operations checked
- [ ] **Memory Management**: RAII used, no manual new/delete
- [ ] **Error Handling**: Errors don't leak sensitive information
- [ ] **Logging**: No sensitive data in logs
- [ ] **Dependencies**: New dependencies justified and reviewed
- [ ] **Tests**: Security-related tests included

### High-Risk Code Areas

Pay extra attention to:
- File I/O operations
- String parsing/manipulation
- Network communication
- Serialization/deserialization
- Configuration loading
- User input processing

### Security Issues in Review

If you find a security issue:

1. **Mark as security concern** in review comment
2. **Explain the risk** and potential impact
3. **Suggest fix** with code example
4. **Block merge** until fixed
5. **Report privately** if exploitable in production

---

## Tools and Automation

### Enabled Security Tools

MetaImGUI uses these automated security tools:

- **CodeQL**: Static analysis for vulnerabilities
- **Dependabot**: Dependency update automation
- **clang-tidy**: Static analysis and modernization
- **ASAN/TSAN/UBSAN**: Runtime error detection

### Manual Security Audits

Perform periodic manual audits:

```bash
# Check for common issues
grep -r "strcpy\|strcat\|sprintf" src/
grep -r "new \|delete " src/
grep -r "malloc\|free" src/

# Review file operations
grep -r "fopen\|ifstream\|ofstream" src/

# Check for command execution
grep -r "system\|exec\|popen" src/
```

---

## Additional Resources

- [CWE Top 25 Most Dangerous Software Weaknesses](https://cwe.mitre.org/top25/)
- [OWASP C++ Security Cheat Sheet](https://cheatsheetseries.owasp.org/)
- [SEI CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682)
- [C++ Core Guidelines - Safety Profile](https://isocpp.github.io/CppCoreGuidelines/)

---

## Questions?

If you're unsure about security implications of your code:
1. Ask in code review
2. Consult security documentation
3. Run sanitizers and static analysis
4. When in doubt, use safer alternatives

**Remember**: It's always better to ask than to introduce a vulnerability!

