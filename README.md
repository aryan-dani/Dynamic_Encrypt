# 🔐 DynamicEncrypt

**A secure file vault application with pluggable cryptographic modules**

DynamicEncrypt is a desktop application built with Qt 6 and modern C++20 that demonstrates advanced software engineering concepts including template metaprogramming, plugin architecture, RAII patterns, and secure memory management.

[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.10-green.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.21+-orange.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

---

## 📋 Table of Contents

- [Features](#-features)
- [Quick Start](#-quick-start)
- [Building from Source](#-building-from-source)
- [Usage Guide](#-usage-guide)
- [Architecture](#-architecture)
- [Plugin Development](#-plugin-development)
- [Security Considerations](#-security-considerations)
- [Testing](#-testing)
- [Contributing](#-contributing)
- [License](#-license)

---

## ✨ Features

### Core Capabilities

- 🔌 **Runtime Plugin System** - Load cryptographic drivers dynamically
- 🔑 **Templated Key Management** - Type-safe key handling with compile-time guarantees
- 🛡️ **RAII Security** - Automatic memory wiping via `ZeroizingBuffer`
- 💾 **Atomic File Operations** - Safe storage with `QSaveFile`
- 🎨 **Qt 6 GUI** - Modern desktop interface

### C++ Concepts Demonstrated

- **Templates**: Policy-based `CryptoEngine<T>`, tag dispatch with `Key<KeyTag>`
- **Inheritance & Polymorphism**: Abstract `CryptoDriver` with plugin overrides
- **Operator Overloading**: `operator<<` for logging, method overloads
- **Exception Safety**: RAII cleanup, try-catch boundaries
- **Smart Pointers**: `unique_ptr` with custom deleters

---

## 🚀 Quick Start

### Run the Application

```powershell
# Simple - just double-click
run_app.bat

# Or from PowerShell
.\run_app.bat
```

---

## 🏗️ Building from Source

### Prerequisites

- Windows 10/11
- Qt 6.10+ with MinGW
- CMake 3.21+
- Ninja (optional)

### Build Steps

**1. Configure**

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64"
```

**2. Compile**

```powershell
cmake --build build
```

**3. Run**

```powershell
.\run_app.bat
```

Binaries output to `build/bin/`, plugins to `build/bin/plugins/`.

---

## 📖 Usage Guide

### Encrypting a File

1. Click **"Generate Key"** → Select 256 bits → **Generate** → **OK**
2. Click **"Add File"** → Choose any file
3. Select the file in "Pending Files"
4. Click **"Encrypt"**
5. ✅ Encrypted! Stored in `Documents/DynamicEncryptVault/`

### Decrypting a File

1. Ensure you have the correct key loaded
2. Select encrypted file in "Vault Entries"
3. Click **"Decrypt"**
4. Choose save location
5. ✅ Original file restored!

---

## 🏛️ Architecture

```
src/
├── core/              # Platform-independent logic
│   ├── CryptoDriver.h     # Plugin interface
│   ├── Key.h              # Template key container
│   ├── ZeroizingBuffer.h  # RAII secure memory
│   ├── Storage.h          # File I/O with overloads
│   └── VaultManager.*     # Plugin orchestration
├── gui/               # Qt Widgets UI
│   ├── MainWindow.*
│   └── KeyDialog.*
├── plugins/
│   └── aes_plugin/    # Demo XOR cipher (replace with real crypto!)
└── main.cpp
```

---

## 🔌 Plugin Development

### Minimal Plugin Example

```cpp
class MyPlugin : public QObject, public dynamicencrypt::core::CryptoDriver {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID CryptoDriver_iid FILE "plugin.json")
    Q_INTERFACES(dynamicencrypt::core::CryptoDriver)
public:
    QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key) override {
        // Use libsodium, OpenSSL, Botan, etc.
    }
    QString name() const override { return "MyPlugin"; }
    QString version() const override { return "1.0"; }
};
```

Deploy the compiled `.dll` to `build/bin/plugins/`.

---

## 🛡️ Security Considerations

### ⚠️ THIS IS A DEMO

The included `aes_plugin` uses **XOR** and is **NOT SECURE** for production.

### Production Checklist

- [ ] Replace XOR with AES-GCM or ChaCha20-Poly1305
- [ ] Use libsodium, OpenSSL, or Botan
- [ ] Implement key derivation (PBKDF2/Argon2)
- [ ] Add key wrapping for export
- [ ] Use hardware RNG
- [ ] Security audit before deployment

---

## 🧪 Testing

## 🧪 Testing

```powershell
# Build and run tests
cmake --build build --target core_tests
.\build\bin\core_tests.exe

# Or use CTest
cd build
ctest --output-on-failure
```

Tests validate:

- ✅ Key memory wiping on destruction
- ✅ Storage round-trip integrity
- ✅ Plugin loading
- ✅ Encrypt/decrypt cycles

---

## 🤝 Contributing

1. Fork the repo
2. Create feature branch (`git checkout -b feature/my-feature`)
3. Add tests for new functionality
4. Commit changes (`git commit -m "Add feature"`)
5. Push and create Pull Request

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

---

## 🙏 Acknowledgments

- Qt Framework
- Catch2 testing library
- Inspired by libsodium and OpenSSL best practices

---

<p align="center">
  <strong>⭐ Star this repo if helpful!</strong><br>
  Built with C++20 and Qt 6
</p>
