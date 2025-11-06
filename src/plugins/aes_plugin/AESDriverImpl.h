#pragma once

#include "core/CryptoDriver.h"
#include "core/ZeroizingBuffer.h"

#include <QObject>

namespace dynamicencrypt::plugins
{

    // Educational AES driver stub demonstrating overriding + plugin wiring.
    class AESDriverImpl : public QObject, public dynamicencrypt::core::CryptoDriver
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID CryptoDriver_iid FILE "plugin.json")
        Q_INTERFACES(dynamicencrypt::core::CryptoDriver)

    public:
        AESDriverImpl() = default;
        ~AESDriverImpl() override = default;

        QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key) override;
        QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key) override;

        QByteArray encrypt(const QByteArray &plaintext, const QString &keyMetadata) override;
        QByteArray decrypt(const QByteArray &ciphertext, const QString &keyMetadata) override;

        QString name() const override;
        QString version() const override;

    private:
        QByteArray xorSeal(const QByteArray &input, const QByteArray &key, const QByteArray &nonce) const;
        QByteArray deriveKeyFromMetadata(const QString &metadata) const;
    };

} // namespace dynamicencrypt::plugins
