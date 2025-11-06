#include "AESDriverImpl.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLatin1Char>
#include <QRandomGenerator>

#include <stdexcept>

namespace dynamicencrypt::plugins
{

    namespace
    {
        constexpr int kNonceSize = 12;
    }

    QByteArray AESDriverImpl::encrypt(const QByteArray &plaintext, const QByteArray &key)
    {
        if (key.isEmpty())
        {
            throw std::invalid_argument("Key must not be empty");
        }
        QByteArray nonce(kNonceSize, Qt::Uninitialized);
        auto *rng = QRandomGenerator::system();
        for (int i = 0; i < nonce.size(); ++i)
        {
            nonce[i] = static_cast<char>(rng->generate());
        }
        QByteArray cipher = xorSeal(plaintext, key, nonce);
        QByteArray output;
        output.reserve(nonce.size() + cipher.size());
        output.append(nonce);
        output.append(cipher);

        // TODO: Replace xorSeal with AES-GCM/ChaCha20-Poly1305 when linking real crypto library.
        //       Nonce rules: never reuse the same nonce + key pair.

        return output;
    }

    QByteArray AESDriverImpl::decrypt(const QByteArray &ciphertext, const QByteArray &key)
    {
        if (ciphertext.size() < kNonceSize)
        {
            throw std::invalid_argument("Ciphertext too short");
        }
        QByteArray nonce = ciphertext.left(kNonceSize);
        QByteArray body = ciphertext.mid(kNonceSize);
        return xorSeal(body, key, nonce);
    }

    QByteArray AESDriverImpl::encrypt(const QByteArray &plaintext, const QString &keyMetadata)
    {
        QByteArray key = deriveKeyFromMetadata(keyMetadata);
        return encrypt(plaintext, key);
    }

    QByteArray AESDriverImpl::decrypt(const QByteArray &ciphertext, const QString &keyMetadata)
    {
        QByteArray key = deriveKeyFromMetadata(keyMetadata);
        return decrypt(ciphertext, key);
    }

    QString AESDriverImpl::name() const
    {
        return QStringLiteral("Demo AES (XOR placeholder)");
    }

    QString AESDriverImpl::version() const
    {
        return QStringLiteral("0.1-demo");
    }

    QByteArray AESDriverImpl::xorSeal(const QByteArray &input, const QByteArray &key, const QByteArray &nonce) const
    {
        QByteArray result(input.size(), Qt::Uninitialized);
        dynamicencrypt::core::ZeroizingBuffer mask(input.size());
        QByteArray &maskBytes = mask.writable();
        maskBytes.resize(input.size());
        for (int i = 0; i < input.size(); ++i)
        {
            const unsigned char keyByte = static_cast<unsigned char>(key.at(i % key.size()));
            const unsigned char nonceByte = static_cast<unsigned char>(nonce.at(i % nonce.size()));
            maskBytes[i] = static_cast<char>(keyByte ^ nonceByte);
            result[i] = static_cast<char>(static_cast<unsigned char>(input.at(i)) ^ static_cast<unsigned char>(maskBytes.at(i)));
        }
        return result;
    }

    QByteArray AESDriverImpl::deriveKeyFromMetadata(const QString &metadata) const
    {
        if (metadata.trimmed().isEmpty())
        {
            throw std::invalid_argument("metadata is empty; cannot derive key");
        }
        if (metadata.trimmed().startsWith(QLatin1Char('{')))
        {
            const auto json = QJsonDocument::fromJson(metadata.toUtf8());
            if (!json.isObject())
            {
                throw std::invalid_argument("invalid key metadata JSON");
            }
            const auto base64 = json.object().value(QStringLiteral("key"));
            if (!base64.isString())
            {
                throw std::invalid_argument("metadata missing 'key' field");
            }
            return QByteArray::fromBase64(base64.toString().toUtf8());
        }
        return QByteArray::fromBase64(metadata.toUtf8());
    }

} // namespace dynamicencrypt::plugins
