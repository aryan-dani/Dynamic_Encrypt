#pragma once

#include "core/Key.h"

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <memory>

namespace dynamicencrypt::gui
{

    class KeyDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit KeyDialog(QWidget *parent = nullptr);

        bool hasKey() const noexcept { return static_cast<bool>(m_key); }
        dynamicencrypt::core::Key<dynamicencrypt::core::SymmetricKeyTag> takeKey();

        void accept() override;

    private slots:
        void onGenerateClicked();
        void onBrowseClicked();
        void onDeriveClicked();

    private:
        void updateStatus(const QString &text);

        QComboBox *m_bitsCombo{nullptr};
        QLineEdit *m_fileLine{nullptr};
        QLineEdit *m_passphraseLine{nullptr};
        QLabel *m_status{nullptr};

        std::unique_ptr<dynamicencrypt::core::Key<dynamicencrypt::core::SymmetricKeyTag>> m_key;
    };

} // namespace dynamicencrypt::gui
