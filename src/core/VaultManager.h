#pragma once

#include "CryptoDriver.h"
#include "Key.h"
#include "Storage.h"
#include "VaultEntry.h"

#include <QDir>
#include <QObject>
#include <QPluginLoader>

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace dynamicencrypt::core
{

    class VaultManager : public QObject
    {
        Q_OBJECT
    public:
        explicit VaultManager(QObject *parent = nullptr);
        ~VaultManager() override;

        void discoverPlugins(const QStringList &searchPaths);
        std::vector<CryptoDriver *> drivers() const;

        void setStorageDirectory(QString path);
        const QString &storageDirectory() const noexcept { return m_storageDir; }

        template <typename DriverType>
        class CryptoEngine
        {
        public:
            explicit CryptoEngine(DriverType *driver) : m_driver(driver)
            {
                static_assert(std::is_base_of_v<CryptoDriver, DriverType>, "DriverType must derive from CryptoDriver");
                if (!driver)
                {
                    throw std::invalid_argument("CryptoEngine requires non-null driver");
                }
            }

            template <typename KeyTag>
            QByteArray encryptWith(const QByteArray &plaintext, const Key<KeyTag> &key)
            {
                static_assert(std::is_same_v<KeyTag, SymmetricKeyTag> || std::is_same_v<KeyTag, AsymmetricKeyTag>,
                              "Unsupported key tag");
                return m_driver->encrypt(plaintext, key.raw());
            }

            template <typename KeyTag>
            QByteArray decryptWith(const QByteArray &ciphertext, const Key<KeyTag> &key)
            {
                static_assert(std::is_same_v<KeyTag, SymmetricKeyTag> || std::is_same_v<KeyTag, AsymmetricKeyTag>,
                              "Unsupported key tag");
                return m_driver->decrypt(ciphertext, key.raw());
            }

            DriverType *driver() const noexcept { return m_driver; }

        private:
            DriverType *m_driver;
        };

        template <typename KeyTag>
        QByteArray encryptWith(CryptoDriver *driver, const QByteArray &plaintext, const Key<KeyTag> &key)
        {
            static_assert(std::is_same_v<KeyTag, SymmetricKeyTag>, "encryptWith currently accepts symmetric keys");
            if (!driver)
            {
                throw std::invalid_argument("driver is null");
            }
            return driver->encrypt(plaintext, key.raw());
        }

       ptoDriver *driver, const QByteArray &ciphertext, const Key<KeyTag> &key)
        {
            static_assert(std::is_same_v<KeyTag, SymmetricKeyTag>, "decryptWith currently accepts symmetric keys");
            if (!driver)
            {
                throw std::invalid_argument("driver is null");
            }
            return driver->decrypt(ciphertext, key.raw());
        }

        QByteArray encryptSymmetric(CryptoDriver *driver, const QByteArray &plaintext, const Key<SymmetricKeyTag> &key,
                                    QByteArray *nonceOut = nullptr);
        QByteArray decryptSymmetric(CryptoDriver *driver, const QByteArray &ciphertext, const Key<SymmetricKeyTag> &key);

        void addEntry(VaultEntry entry);
        const std::vector<VaultEntry> &entries() const noexcept { return m_entries; }

        Storage &storage() noexcept { return m_storage; }

    private:
        struct PluginHolder
        {
            std::unique_ptr<QPluginLoader> loader;
            CryptoDriver *instance{nullptr};
        };

        std::vector<PluginHolder> m_plugins;
        std::vector<VaultEntry> m_entries;
        QString m_storageDir;
        Storage m_storage;
    };

} // namespace dynamicencrypt::core
