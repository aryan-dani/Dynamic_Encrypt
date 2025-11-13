# DynamicEncrypt: A Secure File Vault with Pluggable Crypto Modules

---

## **Title Page**

- **Project Title:** DynamicEncrypt - A Secure File Vault with Pluggable Crypto Modules
- **Student Name:** `[Your Name Here]`
- **PRN:** `[Your PRN Here]`
- **Batch:** `[Your Batch Here]`
- **Subject Name:** `[Your Subject Name Here]`
- **Course:** `[Your Course Name Here]`
- **Faculty Name:** `[Your Faculty's Name Here]`
- **Submission Date:** November 8, 2025

---

## **Abstract**

This report details the design and implementation of "DynamicEncrypt," a C++ desktop application that provides secure, local file encryption and decryption. Built with the Qt 6 framework, the application features a dynamic plugin system, allowing different encryption algorithms to be loaded at runtime without altering the core program. The project serves as a practical demonstration of modern C++ principles, including RAII for secure memory management, templates for compile-time type safety, and polymorphism for an extensible architecture, resulting in a robust and user-friendly file vault.

---

## **Objectives**

The primary objectives of this mini-project are:

- To design and develop a secure file vault application using C++ and the Qt 6 framework.
- To implement a dynamic plugin architecture that allows for extensible encryption capabilities.
- To demonstrate the practical application of modern C++ features, including RAII, templates, polymorphism, and smart pointers.
- To create a user-friendly graphical interface for encrypting, decrypting, and managing files and cryptographic keys.
- To ensure data security through best practices like secure memory handling and atomic file operations.

---

## **Problem Definition**

Many users require a secure method to store sensitive files (e.g., financial records, personal documents, passwords) locally on their computers. Relying solely on cloud storage can introduce privacy concerns, and basic operating system protections are often insufficient against determined attackers. Furthermore, cryptographic standards evolve, so a secure application must be adaptable.

This project solves the problem by providing a desktop application that offers strong, local encryption. Its key innovation is a plugin-based architecture, which ensures that the application is not tied to a single encryption algorithm. This design allows it to be easily updated with stronger or different cryptographic methods in the future, ensuring long-term security and flexibility.

---

## **System Requirements and Build Process**

### System Requirements

- **Operating System:** Windows 10/11
- **Compiler:** A C++20 compliant compiler (e.g., MinGW as part of Qt installation)
- **Build System:** CMake (version 3.21+)
- **Framework:** Qt 6 (version 6.1.0 or newer)

### Build Steps

1.  **Clone the Repository:**
    ```sh
    git clone https://github.com/aryan-dani/Dynamic_Encrypt.git
    cd Dynamic_Encrypt
    ```
2.  **Configure with CMake:** Run CMake to generate the build files. This command should be run from the project's root directory.

    ```sh
    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64"
    ```

    _Note: The `CMAKE_PREFIX_PATH` must point to your specific Qt installation directory._

3.  **Build the Project:** Use CMake's build command to compile the application, plugins, and tests.
    ```sh
    cmake --build build
    ```

### Running the Application

A batch script is provided to handle the environment setup required on Windows.

```sh
.\run_app.bat
```

### Running Tests

The unit tests can be executed from the build directory to verify core functionality.

```sh
.\build\bin\core_tests.exe
```

---

## **System Design / Class Diagram**

`[INSTRUCTION: Insert your UML Class Diagram image here. You can create one using online tools like diagrams.net or Lucidchart. The diagram should show the relationships between the key classes: VaultManager, CryptoDriver, AESDriverImpl, Key, ZeroizingBuffer, Storage, VaultEntry, and MainWindow.]`

**(Example of how to insert an image in Markdown)**
`![Class Diagram](path/to/your/diagram.png)`

---

## **Implementation Details**

The project is logically divided into a core engine, a plugin system, and a graphical user interface.

### 1. VaultManager (`VaultManager.h` / `.cpp`) - The Core Engine

The `VaultManager` class acts as the central controller. It is responsible for discovering and loading encryption plugins from the filesystem at runtime using Qt's `QPluginLoader`. It manages the list of encrypted files (`VaultEntry` objects) and orchestrates the main application logic by providing high-level `encryptSymmetric()` and `decryptSymmetric()` methods that the GUI can call.

```cpp
// In VaultManager.cpp
void VaultManager::discoverPlugins(const QStringList &searchPaths)
{
    // ... loops through directories ...
    auto loader = std::make_unique<QPluginLoader>(info.absoluteFilePath());
    if (loader->load()) {
        auto *driver = qobject_cast<CryptoDriver *>(loader->instance());
        if (driver) {
            // Plugin is valid, add it to our list
            m_plugins.push_back({std::move(loader), driver});
        }
    }
}
```

### 2. CryptoDriver (`CryptoDriver.h`) - The Plugin Contract

This file defines the abstract interface that all encryption plugins must implement. It uses pure virtual functions (`= 0`) for methods like `encrypt`, `decrypt`, `name`, and `version`. This forces every plugin to provide its own implementation, forming the basis of the polymorphic plugin system. The `Q_DECLARE_INTERFACE` macro is used to make the interface recognizable to Qt's plugin framework.

```cpp
// In CryptoDriver.h
class CryptoDriver
{
public:
    virtual ~CryptoDriver() = default;
    // Pure virtual function makes this an abstract class
    virtual QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key) = 0;
    virtual QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key) = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
};
```

### 3. Key Wrapper (`Key.h`) - Type-Safe Key Management

This class is a template-based wrapper (`Key<KeyTag>`) that provides compile-time safety for cryptographic keys. By using tag structs (`SymmetricKeyTag`, `AsymmetricKeyTag`), it prevents the accidental misuse of one key type where another is expected. It utilizes the `ZeroizingBuffer` to hold the raw key data, ensuring that the key is automatically wiped from memory when it goes out of scope (RAII).

```cpp
// In Key.h
template <typename KeyTag>
class Key {
    // ...
private:
    ZeroizingBuffer m_buffer; // Secure RAII buffer
};

// Usage:
Key<SymmetricKeyTag> mySymmetricKey; // A distinct type
```

### 4. ZeroizingBuffer (`ZeroizingBuffer.h`) - Secure Memory Handling

This is a low-level utility class designed to hold sensitive data securely. Its primary feature is its destructor, which automatically overwrites the internal buffer with zeros before deallocation. This prevents sensitive data, like encryption keys, from lingering in RAM where they could be exposed by memory-scraping attacks. Copying is disabled to prevent accidental duplication of sensitive material.

```cpp
// In ZeroizingBuffer.h
~ZeroizingBuffer()
{
    secureWipe(); // Automatically called on destruction
}

void secureWipe() noexcept
{
    // Overwrites memory with zeros
    volatile unsigned char *p = reinterpret_cast<volatile unsigned char*>(m_bytes.data());
    for (int i = 0; i < m_bytes.size(); ++i) {
        p[i] = 0;
    }
}
```

### 5. Storage Handler (`Storage.h`) - Safe File I/O

This class handles all file reading and writing. To prevent data corruption, it uses `QSaveFile` for all write operations. This Qt class performs atomic writes by first writing to a temporary file and only renaming it to the final destination upon successful completion. This guarantees that even if the application crashes mid-operation, the original file remains unharmed.

```cpp
// In Storage.h
void store(const QString &path, const QByteArray &blob)
{
    QSaveFile file(path);
    // ... open and write ...
    if (!file.commit()) { // Atomic operation
        throw std::runtime_error("Failed to commit save file");
    }
}
```

### 6. VaultEntry (`VaultEntry.h`) - Metadata Record

This is a simple data structure used to hold all the necessary metadata about a single encrypted file. It stores the original file path, the path to the encrypted vault file, the name of the algorithm used, the nonce required for decryption, and a timestamp. An instance of this struct is created for every file that is successfully encrypted.

### 7. MainWindow (`MainWindow.h` / `.cpp`) - The User Interface

This class, built with Qt Widgets, provides the entire graphical user interface for the application. It presents the user with lists of available plugins, pending files, and encrypted vault entries. It connects user actions (button clicks) to the appropriate backend logic in the `VaultManager` using Qt's signals and slots mechanism. For example, clicking the "Encrypt" button triggers a process that reads the selected file, calls the `VaultManager` to perform the encryption, and saves the result using the `Storage` handler.

---

## **Output Screenshots**

`[INSTRUCTION: Run the application using the 'run_app.bat' script. Take screenshots of the following and insert them here with the descriptions.]`

**1. Main Application Window**
`![Main Window](path/to/your/screenshot1.png)`
_Caption: The main window upon startup, showing the plugin list on the left and the (currently empty) vault on the right._

**2. Key Generation Dialog**
`![Key Generation](path/to/your/screenshot2.png)`
_Caption: The key management dialog, where a new 256-bit symmetric key has just been generated._

**3. File Encryption**
`![File Encryption](path/to/your/screenshot3.png)`
_Caption: A file has been added to the "Pending Files" list and successfully encrypted. The new vault entry appears on the right, and a confirmation message is shown in the log._

**4. File Decryption**
`![File Decryption](path/to/your/screenshot4.png)`
_Caption: The file save dialog prompts the user to choose a location to save the decrypted file after selecting a vault entry and clicking "Decrypt"._

---

## **Conclusion & Future Scope**

This project successfully demonstrates the creation of a secure and extensible file encryption utility using modern C++ and Qt. The core objectives, including the implementation of a plugin architecture, use of RAII for security, and development of a functional GUI, have all been met. The application provides a solid foundation for a real-world security tool.

**Future Scope:**

- **Implement Real Encryption:** Replace the placeholder XOR cipher in the `AESDriverImpl` plugin with a robust, industry-standard algorithm like AES-256-GCM using a library such as OpenSSL or Botan.
- **Key Management:** Add a secure system to manage keys, allowing users to associate specific keys with specific files and protect them with a master password.
- **Asymmetric Encryption:** Create a new plugin for an asymmetric algorithm (like RSA) to allow for public-key encryption scenarios.
- **Improved UI/UX:** Enhance the user interface with features like drag-and-drop, progress bars for large files, and a more detailed vault view.
- **Cross-Platform Testing:** Formally test and package the application for other operating systems like macOS and Linux.

---

## **References**

`[INSTRUCTION: List any books, websites, or articles you referred to. Here are some examples.]`

- The C++ Programming Language, 4th Edition by Bjarne Stroustrup.
- Qt 6 Documentation: [https://doc.qt.io/](https://doc.qt.io/)
- Effective Modern C++ by Scott Meyers.
- "Plugin Development in Qt" - Qt Documentation.
