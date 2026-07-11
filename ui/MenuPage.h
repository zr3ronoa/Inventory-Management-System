#pragma once

#include <QWidget>

class QTableView;
class QSqlTableModel;
class QSqlRelationalTableModel;
class QPushButton;
class QLabel;

// Restaurant Management > Menu Items + Ingredients.
//
// Top table: MenuItems (simple CRUD).
// Bottom table: IngredientUsage rows for whichever MenuItem is currently
// selected above -- this is where the Many-to-Many relationship between
// MenuItems and Products is actually managed.
class MenuPage : public QWidget
{
    Q_OBJECT

public:
    explicit MenuPage(QWidget *parent = nullptr);

private slots:
    void addMenuItem();
    void deleteMenuItem();
    void saveMenuChanges();
    void revertMenuChanges();

    void onMenuItemSelected();
    void addIngredient();
    void removeIngredient();

private:
    void refreshIngredientsTable();
    int currentMenuId() const; // -1 if nothing selected

    // Menu items (top)
    QTableView *menuTable;
    QSqlTableModel *menuModel;

    // Ingredients for selected menu item (bottom)
    QTableView *ingredientsTable;
    QSqlRelationalTableModel *ingredientsModel;

    QLabel *selectedItemLabel;
    QPushButton *addIngredientButton;
    QPushButton *removeIngredientButton;
};
