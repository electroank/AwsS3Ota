# Contributing to AwsS3Ota

First off, thank you for considering contributing to AwsS3Ota! It's people like you that make this library better for everyone.

## Code of Conduct

This project and everyone participating in it is governed by respect and professionalism. By participating, you are expected to uphold this standard.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When you create a bug report, please include as many details as possible:

- **Use a clear and descriptive title**
- **Describe the exact steps to reproduce the problem**
- **Provide specific examples** - Include code snippets, links to files, or screenshots
- **Describe the behavior you observed** and what behavior you expected
- **Include details about your configuration and environment:**
  - ESP32 board variant (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, etc.)
  - Arduino IDE version or PlatformIO version
  - Library version
  - ESP32 Arduino Core version
  - Operating system

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

- **Use a clear and descriptive title**
- **Provide a step-by-step description** of the suggested enhancement
- **Provide specific examples** to demonstrate the steps
- **Describe the current behavior** and **explain the behavior you expected** instead
- **Explain why this enhancement would be useful** to most users

### Pull Requests

1. **Fork the repo** and create your branch from `main`
2. **Make your changes** following the coding conventions below
3. **Test your changes** on at least one ESP32 board
4. **Update documentation** if you're adding/changing functionality
5. **Write a clear commit message** describing your changes
6. **Submit a pull request!**

## Development Setup

### Prerequisites

- Arduino IDE 2.x or PlatformIO
- ESP32 Arduino Core (latest stable version)
- An ESP32 board for testing

### Local Development

1. Clone your fork:
   ```bash
   git clone https://github.com/YOUR-USERNAME/AwsS3Ota.git
   cd AwsS3Ota
   ```

2. Create a branch:
   ```bash
   git checkout -b feature/my-new-feature
   ```

3. Make your changes and test on real hardware

4. Commit your changes:
   ```bash
   git add .
   git commit -m "Add my new feature"
   git push origin feature/my-new-feature
   ```

## Coding Conventions

### C++ Code Style

- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
  ```cpp
  if (condition) {
      // code
  }
  ```
- **Naming conventions**:
  - Classes: `PascalCase` (e.g., `AwsS3Ota`)
  - Functions/Methods: `camelCase` (e.g., `checkAndUpdate`)
  - Private members: `_camelCase` with underscore prefix (e.g., `_apiBaseUrl`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_RETRIES`)
  - Local variables: `camelCase`

- **Comments**:
  - Use Doxygen-style comments for public APIs
  - Inline comments for complex logic
  - Keep comments up-to-date with code changes

### Documentation

- Update README.md if you add new features
- Add inline documentation for all public methods
- Include examples for new functionality
- Update CHANGELOG section in README.md

### Testing

- Test on multiple ESP32 variants if possible
- Verify examples compile and run correctly
- Test both success and failure scenarios
- Check memory usage (heap) during OTA operations

## Commit Messages

- Use the present tense ("Add feature" not "Added feature")
- Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit the first line to 72 characters or less
- Reference issues and pull requests after the first line

Examples:
```
Add support for custom HTTP headers
Fix memory leak in manifest parsing
Update README with troubleshooting section
```

## Areas We'd Love Help With

- **Testing on different ESP32 variants** (ESP32-S2, S3, C3, C6, H2)
- **Documentation improvements** (especially tutorials and examples)
- **Better error handling and recovery**
- **Additional callback options**
- **Performance optimizations**
- **Unit tests** (if feasible for embedded)
- **Support for other cloud providers** (as separate features)

## Questions?

Feel free to open an issue with the `question` label or contact the maintainer at electroank@gmail.com

## Attribution

This Contributing Guide is adapted from open source contribution guidelines from various successful projects.

---

Thank you for contributing! 🎉
