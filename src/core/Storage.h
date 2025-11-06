#pragma once

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QString>
#include <QtGlobal>

#include <stdexcept>

namespace dynamicencrypt::core
{

    // Storage showcases method overloading for path vs. QFile usage.
    class Storage
    {
    public:
        void store(const QString &path, const QByteArray &blob)
        {
            QSaveFile file(path);
            if (!file.open(QIODevice::WriteOnly))
            {
                throw std::runtime_error(QStringLiteral("Failed to open path for writing: %1").arg(path).toStdString());
            }
            if (file.write(blob) != blob.size())
            {
                throw std::runtime_error("Failed to write entire blob");
            }
            if (!file.commit())
            {
                throw std::runtime_error("Failed to commit save file atomically");
            }
        }

        void store(QFile *file, const QByteArray &blob)
        {
            if (!file)
            {
                throw std::invalid_argument("QFile pointer is null");
            }
            if (!file->isWritable() && !file->open(QIODevice::WriteOnly))
            {
                throw std::runtime_error("QFile not writable");
            }
            if (file->write(blob) != blob.size())
            {
                throw std::runtime_error("Failed to write blob via QFile");
            }
            file->flush();
        }

        QByteArray load(const QString &path)
        {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly))
            {
                throw std::runtime_error(QStringLiteral("Failed to open path for reading: %1").arg(path).toStdString());
            }
            return file.readAll();
        }
    };

} // namespace dynamicencrypt::core
