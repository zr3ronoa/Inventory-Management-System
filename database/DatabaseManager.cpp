#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase(const QString &path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    // SQLite disables foreign key enforcement by default.
    // This must be run on every connection for FK constraints
    // (ON DELETE CASCADE / SET NULL, etc.) to actually work.
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA foreign_keys = ON;");

    createTables();
    seedSampleDataIfEmpty();

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (db.isOpen())
        db.close();
}

void DatabaseManager::createTables()
{
    QSqlQuery query(db);

    // ---------------- INVENTORY MODULE ----------------

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS Categories (
            CategoryID   INTEGER PRIMARY KEY AUTOINCREMENT,
            CategoryName TEXT NOT NULL UNIQUE
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS Suppliers (
            SupplierID   INTEGER PRIMARY KEY AUTOINCREMENT,
            SupplierName TEXT NOT NULL,
            Phone        TEXT,
            Email        TEXT
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS Products (
            ProductID       INTEGER PRIMARY KEY AUTOINCREMENT,
            ProductName     TEXT NOT NULL,
            CategoryID      INTEGER,
            SupplierID      INTEGER,
            QuantityInStock REAL NOT NULL DEFAULT 0,
            Unit            TEXT NOT NULL,
            CostPrice       REAL NOT NULL DEFAULT 0,
            FOREIGN KEY (CategoryID) REFERENCES Categories(CategoryID) ON DELETE SET NULL,
            FOREIGN KEY (SupplierID) REFERENCES Suppliers(SupplierID) ON DELETE SET NULL
        )
    )");

    // ---------------- RESTAURANT MODULE ----------------

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS MenuItems (
            MenuID       INTEGER PRIMARY KEY AUTOINCREMENT,
            ItemName     TEXT NOT NULL,
            SellingPrice REAL NOT NULL
        )
    )");

    // Bridge table implementing the Many-to-Many relationship
    // between MenuItems and Products.
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS IngredientUsage (
            UsageID          INTEGER PRIMARY KEY AUTOINCREMENT,
            MenuID           INTEGER NOT NULL,
            ProductID        INTEGER NOT NULL,
            QuantityRequired REAL NOT NULL,
            FOREIGN KEY (MenuID) REFERENCES MenuItems(MenuID) ON DELETE CASCADE,
            FOREIGN KEY (ProductID) REFERENCES Products(ProductID) ON DELETE CASCADE,
            UNIQUE (MenuID, ProductID)
        )
    )");

    // ---------------- CUSTOMER MODULE ----------------

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS Customers (
            CustomerID INTEGER PRIMARY KEY AUTOINCREMENT,
            Name       TEXT NOT NULL,
            Phone      TEXT
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS Orders (
            OrderID    INTEGER PRIMARY KEY AUTOINCREMENT,
            CustomerID INTEGER NOT NULL,
            Date       TEXT NOT NULL,
            TotalPrice REAL NOT NULL DEFAULT 0,
            Status     TEXT NOT NULL DEFAULT 'Order Received',
            FOREIGN KEY (CustomerID) REFERENCES Customers(CustomerID) ON DELETE CASCADE
        )
    )");

    query.exec(R"(
        CREATE TABLE IF NOT EXISTS OrderItems (
            OrderItemID INTEGER PRIMARY KEY AUTOINCREMENT,
            OrderID     INTEGER NOT NULL,
            MenuID      INTEGER NOT NULL,
            Quantity    INTEGER NOT NULL,
            FOREIGN KEY (OrderID) REFERENCES Orders(OrderID) ON DELETE CASCADE,
            FOREIGN KEY (MenuID) REFERENCES MenuItems(MenuID)
        )
    )");
}

void DatabaseManager::seedSampleDataIfEmpty()
{
    QSqlQuery check(db);
    check.exec("SELECT COUNT(*) FROM Products");
    check.next();
    if (check.value(0).toInt() > 0)
        return; // Already seeded, nothing to do.

    QSqlQuery q(db);

    // Categories
    q.exec("INSERT INTO Categories (CategoryName) VALUES ('Meat')");
    q.exec("INSERT INTO Categories (CategoryName) VALUES ('Dairy')");
    q.exec("INSERT INTO Categories (CategoryName) VALUES ('Bakery')");
    q.exec("INSERT INTO Categories (CategoryName) VALUES ('Vegetables')");

    // Suppliers
    q.exec("INSERT INTO Suppliers (SupplierName, Phone, Email) VALUES "
           "('Fresh Farms Ltd', '9800000001', 'contact@freshfarms.example')");
    q.exec("INSERT INTO Suppliers (SupplierName, Phone, Email) VALUES "
           "('City Bakery Supplies', '9800000002', 'sales@citybakery.example')");

    // Products (ingredients used by the sample menu below)
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Chicken', 1, 1, 20.0, 'kg', 450)");
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Cheese', 2, 1, 10.0, 'kg', 800)");
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Bread', 3, 2, 100, 'pieces', 15)");
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Lettuce', 4, 1, 5.0, 'kg', 120)");
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Tomato', 4, 1, 8.0, 'kg', 100)");
    q.exec("INSERT INTO Products (ProductName, CategoryID, SupplierID, QuantityInStock, Unit, CostPrice) "
           "VALUES ('Potato', 4, 1, 25.0, 'kg', 60)");

    // Menu Items
    q.exec("INSERT INTO MenuItems (ItemName, SellingPrice) VALUES ('Chicken Burger', 350)");
    q.exec("INSERT INTO MenuItems (ItemName, SellingPrice) VALUES ('Cheese Sandwich', 220)");
    q.exec("INSERT INTO MenuItems (ItemName, SellingPrice) VALUES ('French Fries', 150)");

    // IngredientUsage: link MenuItems -> Products (the Many-to-Many relationship)
    // MenuID 1 = Chicken Burger, MenuID 2 = Cheese Sandwich, MenuID 3 = French Fries
    // ProductID 1 = Chicken, 2 = Cheese, 3 = Bread, 4 = Lettuce, 5 = Tomato, 6 = Potato
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (1, 1, 0.2)");  // Chicken
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (1, 2, 0.05)"); // Cheese
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (1, 3, 2)");    // Bread
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (1, 4, 0.05)"); // Lettuce
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (2, 2, 0.08)"); // Cheese
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (2, 3, 2)");    // Bread
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (2, 5, 0.05)"); // Tomato
    q.exec("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (3, 6, 0.3)");  // Potato

    // A sample customer so the "Customer Orders" (staff) page isn't empty on first run.
    q.exec("INSERT INTO Customers (Name, Phone) VALUES ('Walk-in Demo Customer', '9811111111')");
}
