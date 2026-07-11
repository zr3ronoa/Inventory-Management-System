#pragma once

#include <QSqlDatabase>
#include <QString>

// DatabaseManager is a simple singleton responsible for:
//   - Opening the SQLite connection.
//   - Creating all tables (with proper foreign keys) if they do not exist yet.
//   - Seeding the database with sample data the first time it runs, so the
//     application is immediately usable/demoable.
//
// Kept intentionally simple (no ORM, no advanced patterns) since this is a
// beginner-level DBMS course project. All actual queries live in the pages
// that need them (ProductsPage, OrdersPage, etc.) via QSqlTableModel /
// QSqlQuery, using the shared QSqlDatabase connection returned here.
class DatabaseManager
{
public:
    static DatabaseManager& instance();

    // Opens (creating if necessary) the SQLite database file at 'path'
    // and makes sure all tables exist.
    bool openDatabase(const QString &path);

    void closeDatabase();

    QSqlDatabase database() const { return db; }

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void createTables();
    void seedSampleDataIfEmpty();

    QSqlDatabase db;
};
