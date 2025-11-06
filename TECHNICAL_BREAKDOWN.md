# Technical Breakdown of `DynamicEncrypt`

This report details how modern C++ features have been utilized to build a robust, extensible, and secure application.

---

### 1. C++ Templates and Policy-Based Design

Templates are a cornerstone of C++ for writing generic, type-safe code. In this project, we used them to implement a form of **Policy-Based Design** for handling cryptographic keys, ensuring maximum type safety at compile-time.

**Where is it used?**
The primary example is the `Key<T>` class located in `src/core/Key.h`.

**How does it work?**

1.  **Tag-Based Specialization**: We first define simple, empty `struct`s to act as "tags" or "policies":
    ```cpp
    // In src/core/Key.h
    struct SymmetricKeyTag {};
    struct AsymmetricKeyTag {};
    ```
    These tags carry no data; their only purpose is to exist as distinct types.

2.  **Templated `Key` Class**: The `Key` class is templated on a `KeyTag`. This means that `Key<SymmetricKeyTag>` and `Key<AsymmetricKeyTag>` are **completely different and incompatible types** from the compiler's perspective.
    ```cpp
    // In src/core/Key.h
    template <typename KeyTag>
    class Key {
    public:
        // ... constructor, methods ...
    private:
        ZeroizingBuffer m_key_data;
    };
    ```

**Why was it used?**
This design prevents you from accidentally using a symmetric key where an asymmetric one is expected (or vice-versa). Any such attempt will result in a **compile-time error**, which is far better than a difficult-to-debug runtime error. It enforces a strict contract on how keys are used throughout the application.

---

### 2. Inheritance and Polymorphism for a Plugin Architecture

Inheritance is a fundamental pillar of Object-Oriented Programming (OOP) that allows a class to derive properties and behaviors from another. We use it here to create a **pluggable architecture** for cryptographic modules.

**Where is it used?**
The core of this design is the abstract base class `CryptoDriver` and its concrete implementation `AESDriverImpl`.

1.  **Abstract Base Class (`CryptoDriver`)**: `src/core/CryptoDriver.h`
    This file defines the `CryptoDriver` interface. It's an "abstract" class because it contains `pure virtual functions` (e.g., `= 0`). This means it cannot be instantiated on its own; it only defines a contract that other classes must follow.

    ```cpp
    // In src/core/CryptoDriver.h
    class CryptoDriver {
    public:
        virtual ~CryptoDriver() = default;
        virtual QByteArray encrypt(const QByteArray& data, const QByteArray& key) = 0;
        virtual QByteArray decrypt(const QByteArray& data, const QByteArray& key) = 0;
        // ... other pure virtual functions
    };
    ```

2.  **Concrete Implementation (`AESDriverImpl`)**: `src/plugins/aes_plugin/AESDriverImpl.h`
    This class **inherits** from `CryptoDriver` and provides a concrete implementation for all the pure virtual functions. This is what makes it a "plugin."

    ```cpp
    // In src/plugins/aes_plugin/AESDriverImpl.h
    #include "core/CryptoDriver.h"

    class AESDriverImpl : public QObject, public dynamicencrypt::core::CryptoDriver {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "com.arproject.dynamicencrypt.CryptoDriver" FILE "aes_plugin.json")
        Q_INTERFACES(dynamicencrypt::core::CryptoDriver)

    public:
        // ...
        QByteArray encrypt(const QByteArray& data, const QByteArray& key) override;
        QByteArray decrypt(const QByteArray& data, const QByteArray& key) override;
        // ...
    };
    ```
    The `override` keyword is used to explicitly state that we are overriding a base class function, which helps the compiler catch errors.

**Why was it used?**
This powerful design allows the main application to remain completely decoupled from any specific encryption algorithm. The `VaultManager` can load any shared library (`.dll` or `.so`) that implements the `CryptoDriver` interface, discover it at runtime, and use it polymorphically without knowing its concrete type. This makes the application incredibly extensible.

---

### 3. Secure File Handling (I/O)

Proper file handling is critical, especially when dealing with sensitive data. The project uses Qt's robust file I/O classes to ensure data is written safely and atomically.

**Where is it used?**
All file operations are centralized in the `Storage` utility class in `src/core/Storage.h` and `src/core/Storage.cpp`.

**How does it work?**
The `Storage::store` method uses `QSaveFile`, a special Qt class designed to prevent data loss if the application crashes during a write operation.

```cpp
// In src/core/Storage.cpp
bool Storage::store(const QString& path, const QByteArray& data) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open QSaveFile for writing:" << file.errorString();
        return false;
    }

    if (file.write(data) == -1) {
        qWarning() << "Failed to write data to file:" << file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        qWarning() << "Failed to commit file:" << file.errorString();
        return false;
    }

    return true;
}
```
`QSaveFile` works by writing to a temporary file first. Only when the `commit()` method is called successfully is the temporary file renamed to the final destination file. This ensures the original file is never left in a corrupted, partially-written state.

**Why was it used?**
For an application handling encrypted data, data integrity is paramount. Using `QSaveFile` guarantees that file-write operations are **atomic**, preventing corruption of the user's encrypted vault.

---

### 4. Function and Operator Overloading

Overloading allows multiple functions or methods with the same name but different parameters to coexist. This improves code readability and flexibility.

**Where is it used?**

1.  **Method Overloading**: `src/core/Storage.h`
    The `Storage` class provides two versions of the `store` method:
    ```cpp
    // In src/core/Storage.h
    class Storage {
    public:
        // Overload 1: Takes a file path
        static bool store(const QString& path, const QByteArray& data);

        // Overload 2: Takes an already-open QFile object
        static bool store(QFile* f, const QByteArray& data);
    };
    ```
    This provides flexibility to the caller. They can either provide a path and let the function handle opening/closing the file, or manage the file object themselves and pass it in.

2.  **Operator Overloading**: `src/core/Key.h`
    To prevent accidental logging of raw key data (a major security risk), we overloaded the stream insertion operator (`<<`) for the `Key` class.

    ```cpp
    // In src/core/Key.h
    template <typename KeyTag>
    QDebug operator<<(QDebug debug, const Key<KeyTag>& key) {
        QDebugStateSaver saver(debug);
        debug.nospace() << "Key<" << typeid(KeyTag).name() << ">("
                        << "size=" << key.size()
                        << ", content=<REDACTED>"
                        << ")";
        return debug;
    }
    ```
    Now, if a developer tries to log a `Key` object (e.g., `qDebug() << myKey;`), it won't print the sensitive key bytes. Instead, it will print the safe, redacted message: `Key<...>(size=32, content=<REDACTED>)`.

**Why was it used?**
- **Method overloading** provides a more convenient and intuitive API.
- **Operator overloading** enhances security by creating a "guardrail" that prevents sensitive data from being leaked into logs.
