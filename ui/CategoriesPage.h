#pragma once

#include <QWidget>

class QTableView;
class QSqlTableModel;
class QPushButton;

// Inventory > Categories. Simple single-table CRUD (the "1" side of the
// One-to-Many relationship with Products).
class CategoriesPage : public QWidget
{
    Q_OBJECT

public:
    explicit CategoriesPage(QWidget *parent = nullptr);

private slots:
    void addCategory();
    void deleteCategory();
    void saveChanges();
    void revertChanges();

private:
    QTableView *table;
    QSqlTableModel *model;
};
