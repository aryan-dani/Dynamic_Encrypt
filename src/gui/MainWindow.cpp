#include "MainWindow.h"

#include "KeyDialog.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

#include <exception>
#include <memory>
#include <stdexcept>

using dynamicencrypt::core::CryptoDriver;
using dynamicencrypt::core::importSymmetricKey;
using dynamicencrypt::core::Key;
using dynamicencrypt::core::Storage;
using dynamicencrypt::core::SymmetricKeyTag;
using dynamicencrypt::core::VaultEntry;
using dynamicencrypt::core::VaultManager;

namespace dynamicencrypt::gui
{

    MainWindow::MainWindow(VaultManager *manager, QWidget *parent)
        : QMainWindow(parent), m_manager(manager)
    {
        buildUi();
        populatePlugins();
        refreshVaultList();

        m_refreshTimer = new QTimer(this);
        connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshVaultList);
        m_refreshTimer->start(5000);
    }

    void MainWindow::buildUi()
    {
        auto *central = new QWidget(this);
        auto *mainLayout = new QHBoxLayout(central);

        auto *leftLayout = new QVBoxLayout();
        leftLayout->addWidget(new QLabel(QStringLiteral("Available Plugins"), this));
        m_pluginList = new QListWidget(this);
        leftLayout->addWidget(m_pluginList);

        leftLayout->addWidget(new QLabel(QStringLiteral("Pending Files"), this));
        m_pendingList = new QListWidget(this);
        leftLayout->addWidget(m_pendingList);

        m_addButton = new QPushButton(QStringLiteral("Add File"), this);
        leftLayout->addWidget(m_addButton);

        auto *rightLayout = new QVBoxLayout();
        rightLayout->addWidget(new QLabel(QStringLiteral("Vault Entries"), this));
        m_vaultList = new QListWidget(this);
        rightLayout->addWidget(m_vaultList);

        auto *buttonRow = new QHBoxLayout();
        m_encryptButton = new QPushButton(QStringLiteral("Encrypt"), this);
        m_decryptButton = new QPushButton(QStringLiteral("Decrypt"), this);
        m_generateKeyButton = new QPushButton(QStringLiteral("Generate Key"), this);
        m_importKeyButton = new QPushButton(QStringLiteral("Import Key"), this);

        buttonRow->addWidget(m_encryptButton);
        buttonRow->addWidget(m_decryptButton);
        buttonRow->addWidget(m_generateKeyButton);
        buttonRow->addWidget(m_importKeyButton);

        rightLayout->addLayout(buttonRow);

        m_log = new QPlainTextEdit(this);
        m_log->setReadOnly(true);
        rightLayout->addWidget(new QLabel(QStringLiteral("Log"), this));
        rightLayout->addWidget(m_log);

        mainLayout->addLayout(leftLayout, 1);
        mainLayout->addLayout(rightLayout, 2);

        setCentralWidget(central);

        connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddFile);
        connect(m_encryptButton, &QPushButton::clicked, this, &MainWindow::onEncrypt);
        connect(m_decryptButton, &QPushButton::clicked, this, &MainWindow::onDecrypt);
        connect(m_generateKeyButton, &QPushButton::clicked, this, &MainWindow::onGenerateKey);
        connect(m_importKeyButton, &QPushButton::clicked, this, &MainWindow::onImportKey);
    }

    void MainWindow::populatePlugins()
    {
        m_pluginList->clear();
        m_cachedDrivers = m_manager->drivers();
        for (CryptoDriver *driver : m_cachedDrivers)
        {
            QListWidgetItem *item = new QListWidgetItem(QStringLiteral("%1 (%2)")
                                                            .arg(driver->name(), driver->version()));
            m_pluginList->addItem(item);
        }
        statusBar()->showMessage(QStringLiteral("Loaded %1 plugins").arg(m_cachedDrivers.size()));
    }

    void MainWindow::refreshVaultList()
    {
        m_vaultList->clear();
        const auto &entries = m_manager->entries();
        for (const VaultEntry &entry : entries)
        {
            const QString display = QStringLiteral("%1 | %2 | %3")
                                        .arg(QFileInfo(entry.storedPath).fileName())
                                        .arg(entry.algorithm)
                                        .arg(entry.timestamp.toString(Qt::ISODate));
            m_vaultList->addItem(display);
        }
    }

    void MainWindow::logMessage(const QString &message)
    {
        m_log->appendPlainText(QStringLiteral("[%1] %2")
                                   .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                                   .arg(message));
    }

    CryptoDriver *MainWindow::selectedDriver() const
    {
        const int row = m_pluginList->currentRow();
        if (row < 0 || row >= static_cast<int>(m_cachedDrivers.size()))
        {
            return nullptr;
        }
        return m_cachedDrivers.at(row);
    }

    void MainWindow::onAddFile()
    {
        const QString file = QFileDialog::getOpenFileName(this, QStringLiteral("Select file to encrypt"));
        if (file.isEmpty())
        {
            return;
        }
        m_pendingList->addItem(file);
        logMessage(QStringLiteral("Queued file %1").arg(file));
    }

    void MainWindow::onEncrypt()
    {
        CryptoDriver *driver = selectedDriver();
        if (!driver)
        {
            QMessageBox::warning(this, QStringLiteral("No plugin"),
                                 QStringLiteral("Select a crypto plugin first."));
            return;
        }
        if (!m_activeKey)
        {
            QMessageBox::warning(this, QStringLiteral("No key"),
                                 QStringLiteral("Generate or import a symmetric key first."));
            return;
        }
        QListWidgetItem *item = m_pendingList->currentItem();
        if (!item)
        {
            QMessageBox::information(this, QStringLiteral("No file"),
                                     QStringLiteral("Add a file and select it before encrypting."));
            return;
        }
        const QString inputPath = item->text();
        try
        {
            QByteArray blob = m_manager->storage().load(inputPath);
            QByteArray nonce;
            QByteArray cipher = m_manager->encryptSymmetric(driver, blob, *m_activeKey, &nonce);
            const QString vaultFileName = QFileInfo(inputPath).fileName() + QStringLiteral(".vault");
            const QString outputPath = QDir(m_manager->storageDirectory()).filePath(vaultFileName);
            m_manager->storage().store(outputPath, cipher);

            VaultEntry entry;
            entry.originalPath = inputPath;
            entry.storedPath = outputPath;
            entry.algorithm = driver->name();
            entry.nonce = nonce;
            entry.timestamp = QDateTime::currentDateTimeUtc();
            m_manager->addEntry(entry);
            delete m_pendingList->takeItem(m_pendingList->row(item));
            refreshVaultList();
            logMessage(QStringLiteral("Encrypted %1 using %2 -> %3")
                           .arg(inputPath, driver->name(), outputPath));
        }
        catch (const std::exception &ex)
        {
            QMessageBox::critical(this, QStringLiteral("Encryption failed"), QString::fromUtf8(ex.what()));
        }
    }

    void MainWindow::onDecrypt()
    {
        const int row = m_vaultList->currentRow();
        if (row < 0 || row >= static_cast<int>(m_manager->entries().size()))
        {
            QMessageBox::information(this, QStringLiteral("Select entry"),
                                     QStringLiteral("Choose a vault entry to decrypt."));
            return;
        }
        CryptoDriver *driver = selectedDriver();
        if (!driver)
        {
            QMessageBox::warning(this, QStringLiteral("No plugin"),
                                 QStringLiteral("Select the plugin used for encryption."));
            return;
        }
        if (!m_activeKey)
        {
            QMessageBox::warning(this, QStringLiteral("No key"),
                                 QStringLiteral("Load the symmetric key before decrypting."));
            return;
        }
        const VaultEntry &entry = m_manager->entries().at(row);
        const QString savePath = QFileDialog::getSaveFileName(this, QStringLiteral("Save decrypted file"),
                                                              QFileInfo(entry.originalPath).fileName());
        if (savePath.isEmpty())
        {
            return;
        }
        try
        {
            QByteArray blob = m_manager->storage().load(entry.storedPath);
            QByteArray plain = m_manager->decryptSymmetric(driver, blob, *m_activeKey);
            QFile out(savePath);
            if (!out.open(QIODevice::WriteOnly))
            {
                throw std::runtime_error("Failed to open output file");
            }
            out.write(plain);
            out.flush();
            logMessage(QStringLiteral("Decrypted %1 -> %2").arg(entry.storedPath, savePath));
        }
        catch (const std::exception &ex)
        {
            QMessageBox::critical(this, QStringLiteral("Decryption failed"), QString::fromUtf8(ex.what()));
        }
    }

    void MainWindow::onGenerateKey()
    {
        KeyDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted && dialog.hasKey())
        {
            try
            {
                auto key = dialog.takeKey();
                m_activeKey = std::make_unique<Key<SymmetricKeyTag>>(std::move(key));
                statusBar()->showMessage(QStringLiteral("Active key ready (%1 bits)").arg(m_activeKey->size() * 8));
                logMessage(QStringLiteral("Generated new symmetric key."));
            }
            catch (const std::exception &ex)
            {
                QMessageBox::critical(this, QStringLiteral("Key error"), QString::fromUtf8(ex.what()));
            }
        }
    }

    void MainWindow::onImportKey()
    {
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Import key from file"));
        if (path.isEmpty())
        {
            return;
        }
        try
        {
            auto key = importSymmetricKey(path);
            m_activeKey = std::make_unique<Key<SymmetricKeyTag>>(std::move(key));
            statusBar()->showMessage(QStringLiteral("Imported key (%1 bits)").arg(m_activeKey->size() * 8));
            logMessage(QStringLiteral("Imported key from %1").arg(path));
        }
        catch (const std::exception &ex)
        {
            QMessageBox::critical(this, QStringLiteral("Import failed"), QString::fromUtf8(ex.what()));
        }
    }

} // namespace dynamicencrypt::gui
