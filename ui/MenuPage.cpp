#include "MenuPage.h"
#include "../dialogs/IngredientDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QSqlError>
#include <QSqlQuery>
#include <QHeaderView>
#include <QFont>
#include <QSplitter>
#include <algorithm>

MenuPage::MenuPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Restaurant Management - Menu Items", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // ================= TOP: Menu Items =================
    QWidget *topWidget = new QWidget(splitter);
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *menuToolbar = new QHBoxLayout();
    QPushButton *addMenuButton = new QPushButton("Add Menu Item", topWidget);
    QPushButton *deleteMenuButton = new QPushButton("Delete Selected", topWidget);
    QPushButton *saveMenuButton = new QPushButton("Save Changes", topWidget);
    QPushButton *revertMenuButton = new QPushButton("Revert Changes", topWidget);
    menuToolbar->addWidget(addMenuButton);
    menuToolbar->addWidget(deleteMenuButton);
    menuToolbar->addStretch();
    menuToolbar->addWidget(saveMenuButton);
    menuToolbar->addWidget(revertMenuButton);
    topLayout->addLayout(menuToolbar);

    menuModel = new QSqlTableModel(this);
    menuModel->setTable("MenuItems");
    menuModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    menuModel->select();
    menuModel->setHeaderData(1, Qt::Horizontal, "Item Name");
    menuModel->setHeaderData(2, Qt::Horizontal, "Selling Price");

    menuTable = new QTableView(topWidget);
    menuTable->setModel(menuModel);
    menuTable->horizontalHeader()->setStretchLastSection(true);
    menuTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    menuTable->setColumnHidden(0, true);
    topLayout->addWidget(menuTable);

    splitter->addWidget(topWidget);

    // ================= BOTTOM: Ingredients for selected item =================
    QWidget *bottomWidget = new QWidget(splitter);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    selectedItemLabel = new QLabel("Ingredients: (select a menu item above)", bottomWidget);
    QFont f = selectedItemLabel->font();
    f.setBold(true);
    selectedItemLabel->setFont(f);
    bottomLayout->addWidget(selectedItemLabel);

    QHBoxLayout *ingredientToolbar = new QHBoxLayout();
    addIngredientButton = new QPushButton("Add Ingredient", bottomWidget);
    removeIngredientButton = new QPushButton("Remove Selected Ingredient", bottomWidget);
    addIngredientButton->setEnabled(false);
    removeIngredientButton->setEnabled(false);
    ingredientToolbar->addWidget(addIngredientButton);
    ingredientToolbar->addWidget(removeIngredientButton);
    ingredientToolbar->addStretch();
    bottomLayout->addLayout(ingredientToolbar);

    ingredientsModel = new QSqlRelationalTableModel(this);
    ingredientsModel->setTable("IngredientUsage");
    ingredientsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // Columns: 0 UsageID, 1 MenuID, 2 ProductID, 3 QuantityRequired
    ingredientsModel->setRelation(2, QSqlRelation("Products", "ProductID", "ProductName"));
    ingredientsModel->setFilter("MenuID = -1"); // nothing selected initially
    ingredientsModel->select();
    ingredientsModel->setHeaderData(2, Qt::Horizontal, "Product (Ingredient)");
    ingredientsModel->setHeaderData(3, Qt::Horizontal, "Quantity Required");

    ingredientsTable = new QTableView(bottomWidget);
    ingredientsTable->setModel(ingredientsModel);
    ingredientsTable->setItemDelegate(new QSqlRelationalDelegate(ingredientsTable));
    ingredientsTable->horizontalHeader()->setStretchLastSection(true);
    ingredientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ingredientsTable->setColumnHidden(0, true);
    ingredientsTable->setColumnHidden(1, true);
    ingredientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // edit via dialog only
    bottomLayout->addWidget(ingredientsTable);

    splitter->addWidget(bottomWidget);
    mainLayout->addWidget(splitter, 1);

    // ---- connections ----
    connect(addMenuButton, &QPushButton::clicked, this, &MenuPage::addMenuItem);
    connect(deleteMenuButton, &QPushButton::clicked, this, &MenuPage::deleteMenuItem);
    connect(saveMenuButton, &QPushButton::clicked, this, &MenuPage::saveMenuChanges);
    connect(revertMenuButton, &QPushButton::clicked, this, &MenuPage::revertMenuChanges);
    connect(menuTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MenuPage::onMenuItemSelected);
    connect(addIngredientButton, &QPushButton::clicked, this, &MenuPage::addIngredient);
    connect(removeIngredientButton, &QPushButton::clicked, this, &MenuPage::removeIngredient);
}

int MenuPage::currentMenuId() const
{
    QModelIndexList selected = menuTable->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return -1;
    return menuModel->data(menuModel->index(selected.first().row(), 0)).toInt();
}

void MenuPage::addMenuItem()
{
    int row = menuModel->rowCount();
    menuModel->insertRow(row);
    menuModel->setData(menuModel->index(row, 1), "New Menu Item");
    menuModel->setData(menuModel->index(row, 2), 0.0);
    menuTable->selectRow(row);
    menuTable->edit(menuModel->index(row, 1));
}

void MenuPage::deleteMenuItem()
{
    QModelIndexList selected = menuTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete Menu Item", "Please select a row to delete first.");
        return;
    }
    if (QMessageBox::question(this, "Confirm Delete",
                               "Delete the selected menu item? Its ingredient assignments will also be removed.")
        != QMessageBox::Yes) {
        return;
    }

    std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });
    for (const QModelIndex &index : selected)
        menuModel->removeRow(index.row());
    saveMenuChanges();
    refreshIngredientsTable();
}

void MenuPage::saveMenuChanges()
{
    if (!menuModel->submitAll()) {
        QMessageBox::warning(this, "Save Failed", menuModel->lastError().text());
        menuModel->revertAll();
    } else {
        menuModel->select();
    }
}

void MenuPage::revertMenuChanges()
{
    menuModel->revertAll();
}

void MenuPage::onMenuItemSelected()
{
    int menuId = currentMenuId();
    bool hasSelection = (menuId != -1);
    addIngredientButton->setEnabled(hasSelection);
    removeIngredientButton->setEnabled(hasSelection);

    if (hasSelection) {
        QString itemName = menuModel->data(
            menuModel->index(menuTable->selectionModel()->currentIndex().row(), 1)).toString();
        selectedItemLabel->setText(QString("Ingredients for: %1").arg(itemName));
    } else {
        selectedItemLabel->setText("Ingredients: (select a menu item above)");
    }

    refreshIngredientsTable();
}

void MenuPage::refreshIngredientsTable()
{
    int menuId = currentMenuId();
    ingredientsModel->setFilter(QString("MenuID = %1").arg(menuId == -1 ? -1 : menuId));
    ingredientsModel->select();
}

void MenuPage::addIngredient()
{
    int menuId = currentMenuId();
    if (menuId == -1)
        return;

    IngredientDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QSqlQuery q;
    q.prepare("INSERT INTO IngredientUsage (MenuID, ProductID, QuantityRequired) VALUES (?, ?, ?)");
    q.addBindValue(menuId);
    q.addBindValue(dialog.selectedProductId());
    q.addBindValue(dialog.quantityRequired());

    if (!q.exec()) {
        QMessageBox::warning(this, "Could Not Add Ingredient",
                              "This product may already be assigned to this menu item.\n" + q.lastError().text());
        return;
    }

    refreshIngredientsTable();
}

void MenuPage::removeIngredient()
{
    QModelIndexList selected = ingredientsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Remove Ingredient", "Please select an ingredient row to remove.");
        return;
    }

    int usageId = ingredientsModel->data(ingredientsModel->index(selected.first().row(), 0)).toInt();
    QSqlQuery q;
    q.prepare("DELETE FROM IngredientUsage WHERE UsageID = ?");
    q.addBindValue(usageId);
    q.exec();

    refreshIngredientsTable();
}
