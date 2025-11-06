#include "core/VaultManager.h"
#include "gui/MainWindow.h"

#include <QApplication>
#include <QMessageBox>

#include <exception>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    try
    {
        dynamicencrypt::core::VaultManager manager;
        dynamicencrypt::gui::MainWindow window(&manager);
        window.resize(1000, 600);
        window.show();
        return app.exec();
    }
    catch (const std::exception &ex)
    {
        QMessageBox::critical(nullptr, QStringLiteral("Startup failure"), QString::fromUtf8(ex.what()));
        return EXIT_FAILURE;
    }
}
