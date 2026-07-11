#pragma once

#include <QWidget>

class QTableView;
class QSqlTableModel;

// Inventory > Suppliers. Simple single-table CRUD (the "1" side of the
// One-to-Many relationship with Products).
class SuppliersPage : public QWidget
{
    Q_OBJECT

public:
    explicit SuppliersPage(QWidget *parent = nullptr);

private slots:
    void addSupplier();
    void deleteSupplier();
    void saveChanges();
    void revertChanges();

private:
    QTableView *table;
    QSqlTableModel *model;
};
