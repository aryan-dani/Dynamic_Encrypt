# Contributing to DynamicEncrypt

Thank you for your interest in contributing! 🎉

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR-USERNAME/Dynamic_Encrypt.git`
3. Create a feature branch: `git checkout -b feature/my-awesome-feature`

## Development Setup

```powershell
# Configure with all warnings enabled
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64" -DCMAKE_CXX_FLAGS="/W4"

# Build
cmake --build build

# Run tests
.\build\bin\core_tests.exe
```

## Code Style

- **Indentation**: 4 spaces (no tabs)
- **Naming**:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Member variables: `m_camelCase`
  - Constants: `kPascalCase`
- **Headers**: Use `#pragma once`
- **Includes**: Group Qt, STL, project headers separately

## Commit Guidelines

- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit first line to 72 characters
- Reference issues/PRs in commit body

Example:

```
Add AES-GCM encryption plugin

- Implement libsodium-based AESGCMDriver
- Add nonce generation and auth tag validation
- Include unit tests for encrypt/decrypt cycles

Fixes #42
```

## Pull Request Process

1. Update README.md with details of changes if applicable
2. Add tests for new functionality
3. Ensure all tests pass: `ctest --test-dir build`
4. Update documentation/comments
5. Request review from maintainers

## Testing

All new features must include tests:

```cpp
TEST_CASE("My feature description", "[tag]") {
    // Arrange
    auto fixture = setupTestFixture();

    // Act
    auto result = myNewFeature(fixture);

    // Assert
    REQUIRE(result.isValid());
    CHECK(result.value() == expectedValue);
}
```

## Questions?

Open an issue with the "question" label or start a discussion.

Thanks again! 🚀
