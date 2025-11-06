#include "KeyDialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

#include <stdexcept>

using dynamicencrypt::core::generateSymmetricKey;
using dynamicencrypt::core::importSymmetricKey;
using dynamicencrypt::core::Key;
using dynamicencrypt::core::SymmetricKeyTag;

namespace dynamicencrypt::gui
{

    KeyDialog::KeyDialog(QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(QStringLiteral("Key Management"));
        auto *layout = new QVBoxLayout(this);

        auto *bitsRow = new QHBoxLayout();
        bitsRow->addWidget(new QLabel(QStringLiteral("Symmetric key size (bits):"), this));
        m_bitsCombo = new QComboBox(this);
        m_bitsCombo->addItems({QStringLiteral("128"), QStringLiteral("256")});
        bitsRow->addWidget(m_bitsCombo);
        QPushButton *generateButton = new QPushButton(QStringLiteral("Generate"), this);
        bitsRow->addWidget(generateButton);
        layout->addLayout(bitsRow);

        auto *fileRow = new QHBoxLayout();
        fileRow->addWidget(new QLabel(QStringLiteral("Import from file:"), this));
        m_fileLine = new QLineEdit(this);
        m_fileLine->setReadOnly(true);
        fileRow->addWidget(m_fileLine);
        QPushButton *browseButton = new QPushButton(QStringLiteral("Browse"), this);
        fileRow->addWidget(browseButton);
        layout->addLayout(fileRow);

        auto *passRow = new QHBoxLayout();
        passRow->addWidget(new QLabel(QStringLiteral("Derive from passphrase:"), this));
        m_passphraseLine = new QLineEdit(this);
        m_passphraseLine->setEchoMode(QLineEdit::Password);
        passRow->addWidget(m_passphraseLine);
        QPushButton *deriveButton = new QPushButton(QStringLiteral("Derive"), this);
        passRow->addWidget(deriveButton);
        layout->addLayout(passRow);

        m_status = new QLabel(QStringLiteral("No key loaded"), this);
        layout->addWidget(m_status);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttonBox);

        connect(generateButton, &QPushButton::clicked, this, &KeyDialog::onGenerateClicked);
        connect(browseButton, &QPushButton::clicked, this, &KeyDialog::onBrowseClicked);
        connect(deriveButton, &QPushButton::clicked, this, &KeyDialog::onDeriveClicked);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &KeyDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &KeyDialog::reject);
    }

    Key<SymmetricKeyTag> KeyDialog::takeKey()
    {
        if (!m_key)
        {
            throw std::runtime_error("Key unavailable");
        }
        Key<SymmetricKeyTag> key = std::move(*m_key);
        m_key.reset();
        return key;
    }

    void KeyDialog::accept()
    {
        if (!m_key)
        {
            QMessageBox::warning(this, QStringLiteral("No key"),
                                 QStringLiteral("Generate or import a key before closing."));
            return;
        }
        QDialog::accept();
    }

    void KeyDialog::onGenerateClicked()
    {
        bool ok = false;
        const int bits = m_bitsCombo->currentText().toInt(&ok);
        if (!ok)
        {
            QMessageBox::warning(this, QStringLiteral("Invalid size"), QStringLiteral("Choose a numeric key size."));
            return;
        }
        auto generated = generateSymmetricKey(bits);
        m_key = std::make_unique<Key<SymmetricKeyTag>>(std::move(generated));
        updateStatus(QStringLiteral("Generated %1-bit key").arg(bits));
    }

    void KeyDialog::onBrowseClicked()
    {
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Select key file"));
        if (path.isEmpty())
        {
            return;
        }
        try
        {
            auto key = importSymmetricKey(path);
            m_key = std::make_unique<Key<SymmetricKeyTag>>(std::move(key));
            m_fileLine->setText(path);
            updateStatus(QStringLiteral("Imported key from %1").arg(path));
        }
        catch (const std::exception &ex)
        {
            QMessageBox::critical(this, QStringLiteral("Import failed"), QString::fromUtf8(ex.what()));
        }
    }

    void KeyDialog::onDeriveClicked()
    {
        const QString passphrase = m_passphraseLine->text();
        if (passphrase.isEmpty())
        {
            QMessageBox::information(this, QStringLiteral("Passphrase required"),
                                     QStringLiteral("Enter a passphrase to derive a key."));
            return;
        }
        const int bits = 256;
        try
        {
            auto key = importSymmetricKey(passphrase.toUtf8(), bits);
            m_key = std::make_unique<Key<SymmetricKeyTag>>(std::move(key));
            updateStatus(QStringLiteral("Derived %1-bit key from passphrase").arg(bits));
        }
        catch (const std::exception &ex)
        {
            QMessageBox::critical(this, QStringLiteral("Derivation failed"), QString::fromUtf8(ex.what()));
        }
    }

    void KeyDialog::updateStatus(const QString &text)
    {
        m_status->setText(text);
    }

} // namespace dynamicencrypt::gui
