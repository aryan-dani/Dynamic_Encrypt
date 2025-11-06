#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <QtPlugin>

#include <stdexcept>

namespace dynamicencrypt::core
{

    // Abstract base demonstrates interface + overriding in plugins.
    class CryptoDriver
    {
    public:
        virtual ~CryptoDriver() = default;

        // Base overloads with raw key bytes.
        virtual QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key) = 0;
        virtual QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key) = 0;

        // Overloaded helpers using metadata strings (e.g., key identifiers, KMS handles).
        virtual QByteArray encrypt(const QByteArray &plaintext, const QString &keyMetadata)
        {
            Q_UNUSED(plaintext);
            Q_UNUSED(keyMetadata);
            throw std::runtime_error("encrypt(metadata) not implemented for this driver");
        }

        virtual QByteArray decrypt(const QByteArray &ciphertext, const QString &keyMetadata)
        {
            Q_UNUSED(ciphertext);
            Q_UNUSED(keyMetadata);
            throw std::runtime_error("decrypt(metadata) not implemented for this driver");
        }

        virtual QString name() const = 0;
        virtual QString version() const = 0;
    };

} // namespace dynamicencrypt::core

#define CryptoDriver_iid "com.dynamicencrypt.CryptoDriver"
Q_DECLARE_INTERFACE(dynamicancrypt::core::CryptoDriver, CryptoDriver_iid)
