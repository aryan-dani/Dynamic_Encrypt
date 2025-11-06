#pragma once

#include "core/Key.h"
#include "core/VaultManager.h"

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QListWidget>
#include <QTimer>

#include <memory>

namespace dynamicencrypt::gui
{

    class KeyDialog;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(dynamicencrypt::core::VaultManager *manager, QWidget *parent = nullptr);
        ~MainWindow() override = default;

    private slots:
        void onAddFile();
        void onEncrypt();
        void onDecrypt();
        void onGenerateKey();
        void onImportKey();

    private:
        void buildUi();
        void populatePlugins();
        void refreshVaultList();
        void logMessage(const QString &message);
        dynamicencrypt::core::CryptoDriver *selectedDriver() const;

        dynamicencrypt::core::VaultManager *m_manager{nullptr};
        QListWidget *m_pluginList{nullptr};
        QListWidget *m_vaultList{nullptr};
        QListWidget *m_pendingList{nullptr};
        QPlainTextEdit *m_log{nullptr};
        QPushButton *m_encryptButton{nullptr};
        QPushButton *m_decryptButton{nullptr};
        QPushButton *m_addButton{nullptr};
        QPushButton *m_generateKeyButton{nullptr};
        QPushButton *m_importKeyButton{nullptr};
        QTimer *m_refreshTimer{nullptr};

        std::unique_ptr<dynamicencrypt::core::Key<dynamicencrypt::core::SymmetricKeyTag>> m_activeKey;
        mutable std::vector<dynamicencrypt::core::CryptoDriver *> m_cachedDrivers;
    };

} // namespace dynamicencrypt::gui
