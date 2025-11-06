#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace dynamicencrypt::core
{

    struct VaultEntry
    {
        QString originalPath;
        QString storedPath;
        QString algorithm;
        QByteArray nonce;
        QDateTime timestamp;
    };

} // namespace dynamicencrypt::core
