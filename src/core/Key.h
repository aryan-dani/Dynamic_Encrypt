#pragma once

#include "ZeroizingBuffer.h"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QString>

#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace dynamicencrypt::core
{

    struct SymmetricKeyTag
    {
        static constexpr const char *name = "symmetric";
    };

    struct AsymmetricKeyTag
    {
        static constexpr const char *name = "asymmetric";
    };

    template <typename KeyTag>
    class Key
    {
        static_assert(std::is_same_v<KeyTag, SymmetricKeyTag> || std::is_same_v<KeyTag, AsymmetricKeyTag>,
                      "Unsupported key tag");

    public:
        Key() = default;
        explicit Key(QByteArray data, QString label = {})
            : m_buffer(std::move(data)), m_label(std::move(label))
        {
        }

        Key(const Key &) = delete;
        Key &operator=(const Key &) = delete;

        Key(Key &&other) noexcept = default;
        Key &operator=(Key &&other) noexcept = default;

        ~Key()
        {
            secureWipe();
        }

        const QByteArray &raw() const noexcept { return m_buffer.bytes(); }
        QByteArray materialize() const { return QByteArray(m_buffer.bytes()); }

        int size() const noexcept { return m_buffer.bytes().size(); }
        const QString &label() const noexcept { return m_label; }

        void secureWipe() noexcept
        {
            m_buffer.secureWipe();
        }

        static constexpr const char *tagName() noexcept { return KeyTag::name; }

    private:
        ZeroizingBuffer m_buffer;
        QString m_label;

        template <typename TTag>
        friend std::ostream &operator<<(std::ostream &os, const Key<TTag> &key);
    };

    template <typename KeyTag>
    std::ostream &operator<<(std::ostream &os, const Key<KeyTag> &key)
    {
        os << "Key<" << Key<KeyTag>::tagName() << "> size=" << key.size() * 8 << " bits";
        if (!key.m_label.isEmpty())
        {
            os << " label=" << key.m_label.toStdString();
        }
        return os;
    }

    inline Key<SymmetricKeyTag> generateSymmetricKey(int sizeBits)
    {
        if (sizeBits % 8 != 0 || sizeBits <= 0)
        {
            throw std::invalid_argument("sizeBits must be positive and a multiple of 8");
        }
        QByteArray material(sizeBits / 8, Qt::Uninitialized);
        auto *rng = QRandomGenerator::system();
        for (int i = 0; i < material.size(); ++i)
        {
            material[i] = static_cast<char>(rng->generate());
        }
        return Key<SymmetricKeyTag>(material, QStringLiteral("generated"));
    }

    inline Key<SymmetricKeyTag> importSymmetricKey(const QString &path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            throw std::runtime_error(QStringLiteral("Failed to open key file: %1").arg(path).toStdString());
        }
        QByteArray blob = file.readAll();
        return Key<SymmetricKeyTag>(blob, QStringLiteral("file:%1").arg(path));
    }

    inline Key<SymmetricKeyTag> importSymmetricKey(const QByteArray &passphrase, int sizeBits = 256)
    {
        if (passphrase.isEmpty())
        {
            throw std::invalid_argument("passphrase must not be empty");
        }
        QCryptographicHash hasher(QCryptographicHash::Sha256);
        hasher.addData(passphrase);
        QByteArray digest = hasher.result();
        if (sizeBits > digest.size() * 8)
        {
            digest = QCryptographicHash::hash(digest, QCryptographicHash::Sha256);
        }
        digest.truncate(sizeBits / 8);
        return Key<SymmetricKeyTag>(digest, QStringLiteral("passphrase"));
    }

    inline Key<SymmetricKeyTag> importSymmetricKey(const QJsonObject &metadata)
    {
        const auto base64 = metadata.value(QStringLiteral("key"));
        if (!base64.isString())
        {
            throw std::invalid_argument("metadata missing base64 key");
        }
        QByteArray blob = QByteArray::fromBase64(base64.toString().toUtf8());
        return Key<SymmetricKeyTag>(blob, QStringLiteral("metadata"));
    }

}
