#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include "database/DatabaseManager.h"
#include "ui/MainWindow.h"

// Entry point of the application.
// Responsibilities:
//   1. Start the Qt application.
//   2. Load the app-wide visual theme (resources/style.qss).
//   3. Open (and initialize, if needed) the SQLite database.
//   4. Show the main window.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Restaurant Inventory Management System");

    // Apply the shared stylesheet (embedded via resources/resources.qrc)
    // so every page gets the same modern, card-based look.
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    // Open the database before creating any UI that depends on it.
    if (!DatabaseManager::instance().openDatabase("restaurant.db")) {
        QMessageBox::critical(nullptr, "Database Error",
                               "Could not open the SQLite database. The application will now exit.");
        return -1;
    }

    MainWindow window;
    window.resize(1100, 700);
    window.show();

    return app.exec();
}
