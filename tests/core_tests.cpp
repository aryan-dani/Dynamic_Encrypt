#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/Key.h"
#include "core/Storage.h"
#include "core/VaultManager.h"
#include "core/ZeroizingBuffer.h"

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

#include <algorithm>
#include <memory>

using dynamicencrypt::core::generateSymmetricKey;
using dynamicencrypt::core::Key;
using dynamicencrypt::core::Storage;
using dynamicencrypt::core::SymmetricKeyTag;
using dynamicencrypt::core::VaultManager;
using dynamicencrypt::core::ZeroizingBuffer;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Catch::Session session;
    return session.run(argc, argv);
}

TEST_CASE("Key destructor wipes memory", "[key]")
{
    bool wiped = false;
    ZeroizingBuffer::setOnWipe([&](const QByteArray &payload)
                               {
        wiped = true;
        REQUIRE(std::all_of(payload.begin(), payload.end(), [](char c) { return c == 0; })); });
    {
        auto key = std::make_unique<Key<SymmetricKeyTag>>(QByteArray("demo", 4));
        (void)key;
    }
    ZeroizingBuffer::setOnWipe({});
    REQUIRE(wiped);
}

TEST_CASE("Storage store/load roundtrip", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("blob.bin"));

    Storage storage;
    const QByteArray expected("vault-data");
    storage.store(path, expected);
    const QByteArray readBack = storage.load(path);
    REQUIRE(readBack == expected);
}

TEST_CASE("Plugin encrypt/decrypt roundtrip", "[plugin]")
{
    VaultManager manager;
    manager.discoverPlugins({QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"))});
    auto drivers = manager.drivers();
    REQUIRE_FALSE(drivers.empty());

    auto key = generateSymmetricKey(128);
    const QByteArray plaintext("secret payload");
    QByteArray cipher = manager.encryptSymmetric(drivers.front(), plaintext, key);
    REQUIRE(cipher != plaintext);
    QByteArray recovered = manager.decryptSymmetric(drivers.front(), cipher, key);
    REQUIRE(recovered == plaintext);
}

TEST_CASE("VaultManager plugin discovery", "[vault]")
{
    VaultManager manager;
    manager.discoverPlugins({QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"))});
    REQUIRE(manager.drivers().size() >= 1);
}
