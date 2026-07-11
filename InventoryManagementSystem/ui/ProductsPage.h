#pragma once

#include <QWidget>

class QTableView;
class QSqlRelationalTableModel;
class QLineEdit;
class QPushButton;

// Inventory > Products.
// Demonstrates CRUD + foreign keys using QSqlRelationalTableModel, which
// automatically joins CategoryID -> Categories.CategoryName and
// SupplierID -> Suppliers.SupplierName, and lets the user pick them from
// combo boxes directly inside the table (via QSqlRelationalDelegate).
class ProductsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProductsPage(QWidget *parent = nullptr);

private slots:
    void addProduct();
    void deleteProduct();
    void saveChanges();
    void revertChanges();
    void searchProducts(const QString &text);

private:
    QTableView *table;
    QSqlRelationalTableModel *model;
    QLineEdit *searchBox;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
    QPushButton *revertButton;
};
