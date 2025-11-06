#include "VaultManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QStandardPaths>

#include <QDebug>

namespace dynamicencrypt::core
{

    namespace
    {
        QStringList defaultPluginPaths()
        {
            QStringList paths;
            const QString appDir = QCoreApplication::applicationDirPath();
            paths << QDir(appDir).filePath(QStringLiteral("plugins"));
            paths << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/DynamicEncrypt/plugins");
            return paths;
        }
    }

    VaultManager::VaultManager(QObject *parent)
        : QObject(parent)
    {
        const QString defaultVaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/DynamicEncryptVault");
        setStorageDirectory(defaultVaultDir);
        discoverPlugins(defaultPluginPaths());
    }

    VaultManager::~VaultManager() = default;

    void VaultManager::discoverPlugins(const QStringList &searchPaths)
    {
        m_plugins.clear();
        for (const QString &path : searchPaths)
        {
            QDir dir(path);
            if (!dir.exists())
            {
                continue;
            }
            const QFileInfoList entries = dir.entryInfoList(QDir::Files);
            for (const QFileInfo &info : entries)
            {
                if (!QLibrary::isLibrary(info.absoluteFilePath()))
                {
                    continue;
                }
                auto loader = std::make_unique<QPluginLoader>(info.absoluteFilePath());
                if (!loader->load())
                {
                    qWarning() << "Failed to load plugin" << info.fileName() << loader->errorString();
                    continue;
                }
                QObject *object = loader->instance();
                if (!object)
                {
                    qWarning() << "Plugin instance null for" << info.fileName();
                    continue;
                }
                auto *driver = qobject_cast<CryptoDriver *>(object);
                if (!driver)
                {
                    qWarning() << "Plugin" << info.fileName() << "does not implement CryptoDriver";
                    loader->unload();
                    continue;
                }
                PluginHolder holder;
                holder.loader = std::move(loader);
                holder.instance = driver;
                m_plugins.push_back(std::move(holder));
            }
        }
    }

    std::vector<CryptoDriver *> VaultManager::drivers() const
    {
        std::vector<CryptoDriver *> result;
        result.reserve(m_plugins.size());
        for (const auto &holder : m_plugins)
        {
            result.push_back(holder.instance);
        }
        return result;
    }

    void VaultManager::setStorageDirectory(QString path)
    {
        QDir dir(std::move(path));
        if (!dir.exists())
        {
            dir.mkpath(QStringLiteral("."));
        }
        m_storageDir = dir.absolutePath();
    }

    QByteArray VaultManager::encryptSymmetric(CryptoDriver *driver, const QByteArray &plaintext,
                                              const Key<SymmetricKeyTag> &key, QByteArray *nonceOut)
    {
        QByteArray cipher = encryptWith(driver, plaintext, key);
        if (nonceOut && cipher.size() > 12)
        {
            *nonceOut = cipher.left(12);
        }
        return cipher;
    }

    QByteArray VaultManager::decryptSymmetric(CryptoDriver *driver, const QByteArray &ciphertext,
                                              const Key<SymmetricKeyTag> &key)
    {
        return decryptWith(driver, ciphertext, key);
    }

    void VaultManager::addEntry(VaultEntry entry)
    {
        m_entries.push_back(std::move(entry));
    }

} // namespace dynamicencrypt::core
