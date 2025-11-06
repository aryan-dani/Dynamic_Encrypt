# DynamicEncrypt

Secure file vault prototype with pluggable crypto engines and a Qt 6 desktop UI.

## Features

- **Plugin architecture** powered by Qt's `QPluginLoader` and the `CryptoDriver` interface (`src/core/CryptoDriver.h`).
- **Template-based key safety** using `Key<T>` and policy tags (`SymmetricKeyTag`, `AsymmetricKeyTag`).
- **RAII secrecy** via `ZeroizingBuffer` that wipes memory and notifies tests.
- **Method and operator overloading** (`Storage::store` overloads, `CryptoDriver::encrypt` overloads, `operator<<` for keys).
- **Qt 6 GUI** (`src/gui/MainWindow.*`) showcasing plugin discovery, key management, and vault operations.
- **Demo plugin** (`aes_plugin`) illustrating where to integrate AES-GCM/ChaCha20-Poly1305.
- **Catch2 tests** validating key erasure, storage round-trips, plugin loading, and encryption cycles.

## Building

```powershell
# From repository root
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"
cmake --build build
```

Adjust `CMAKE_PREFIX_PATH` to your Qt 6 installation. The binaries land in `build/bin`, plugins in `build/bin/plugins`.

## Running

```powershell
cd build/bin
# Ensure plugins/ contains aes_plugin.dll (copied automatically by CMake)
./dynamicancrypt.exe
```

The app creates a "DynamicEncryptVault" directory inside your documents folder for vault storage.

## Tests

```powershell
cmake --build build --target core_tests
ctest --test-dir build
```

Tests rely on the built demo plugin and Qt Core only.

## Plugin authoring

1. Implement a class deriving from `QObject` and `CryptoDriver`.
2. Add `Q_PLUGIN_METADATA(IID CryptoDriver_iid FILE "plugin.json")` and `Q_INTERFACES(dynamicancrypt::core::CryptoDriver)`.
3. Ship the resulting shared library into `bin/plugins` next to the executable (or any directory passed to `VaultManager::discoverPlugins`).
4. Implement both raw-key overloads (`encrypt/decrypt` using `QByteArray`) and, optionally, metadata overloads if you leverage KMS identifiers.

### Replacing the demo crypto

The provided `aes_plugin` uses an XOR-based fallback for educational purposes. Replace the body of `AESDriverImpl::xorSeal` with real AES-GCM/ChaCha20-Poly1305 operations using libsodium or OpenSSL (remember to:

- Enforce unique nonces per key, persist nonces alongside ciphertexts, and store authentication tags.
- Wipe intermediate buffers after use (reusing `ZeroizingBuffer`).
- Wrap export/import so raw keys never touch disk unprotected; use PBKDF2/Argon2 and AES-KW where possible.)

## Security notes

- Raw symmetric keys are never written to disk; exports are user-triggered and should be wrapped before production use.
- Temporary files rely on `QSaveFile` for atomic replace; adjust for multi-user environments as needed.
- The demo XOR cipher is **not secure**. Replace with a vetted, authenticated cipher before handling sensitive data.

## Directory layout

```
src/core    // Core logic, templates, interfaces
src/gui     // Qt widgets
src/plugins // Runtime-loadable crypto drivers
tests       // Catch2 unit tests
```

## Troubleshooting

- **Qt not found**: ensure `CMAKE_PREFIX_PATH` includes your Qt 6 installation root containing `lib/cmake/Qt6`.
- **Plugin missing**: verify `bin/plugins/aes_plugin.dll` exists beside the executable. Use `Qt_DEBUG_PLUGINS=1` to trace loading issues.
- **Catch2 download blocked**: pre-fetch Catch2 and set `CMAKE_PREFIX_PATH` accordingly, or vendor the release archive.
