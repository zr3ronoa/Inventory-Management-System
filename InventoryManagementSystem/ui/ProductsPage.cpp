#include "ProductsPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QSqlError>
#include <QHeaderView>
#include <QFont>
#include <algorithm>

ProductsPage::ProductsPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Inventory - Products", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // ---- Toolbar: search + buttons ----
    QHBoxLayout *toolbar = new QHBoxLayout();
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search by product name...");
    addButton = new QPushButton("Add Product", this);
    deleteButton = new QPushButton("Delete Selected", this);
    saveButton = new QPushButton("Save Changes", this);
    revertButton = new QPushButton("Revert Changes", this);

    toolbar->addWidget(searchBox, 1);
    toolbar->addWidget(addButton);
    toolbar->addWidget(deleteButton);
    toolbar->addWidget(saveButton);
    toolbar->addWidget(revertButton);
    mainLayout->addLayout(toolbar);

    // ---- Table backed by a relational model ----
    model = new QSqlRelationalTableModel(this);
    model->setTable("Products");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Column order in the Products table:
    // 0 ProductID, 1 ProductName, 2 CategoryID, 3 SupplierID,
    // 4 QuantityInStock, 5 Unit, 6 CostPrice
    model->setRelation(2, QSqlRelation("Categories", "CategoryID", "CategoryName"));
    model->setRelation(3, QSqlRelation("Suppliers", "SupplierID", "SupplierName"));
    model->select();

    model->setHeaderData(1, Qt::Horizontal, "Product Name");
    model->setHeaderData(2, Qt::Horizontal, "Category");
    model->setHeaderData(3, Qt::Horizontal, "Supplier");
    model->setHeaderData(4, Qt::Horizontal, "Qty In Stock");
    model->setHeaderData(5, Qt::Horizontal, "Unit");
    model->setHeaderData(6, Qt::Horizontal, "Cost Price");

    table = new QTableView(this);
    table->setModel(model);
    // The relational delegate turns CategoryID/SupplierID columns into
    // combo boxes showing the human-readable name, while still storing
    // the correct foreign key underneath.
    table->setItemDelegate(new QSqlRelationalDelegate(table));
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setColumnHidden(0, true); // Hide raw ProductID (surrogate key)
    mainLayout->addWidget(table, 1);

    QLabel *hint = new QLabel(
        "Tip: double-click a cell to edit it directly in the table. "
        "Click 'Save Changes' to commit to the database, or 'Revert Changes' to discard edits.",
        this);
    hint->setWordWrap(true);
    mainLayout->addWidget(hint);

    connect(addButton, &QPushButton::clicked, this, &ProductsPage::addProduct);
    connect(deleteButton, &QPushButton::clicked, this, &ProductsPage::deleteProduct);
    connect(saveButton, &QPushButton::clicked, this, &ProductsPage::saveChanges);
    connect(revertButton, &QPushButton::clicked, this, &ProductsPage::revertChanges);
    connect(searchBox, &QLineEdit::textChanged, this, &ProductsPage::searchProducts);
}

void ProductsPage::addProduct()
{
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 1), "New Product");
    model->setData(model->index(row, 4), 0.0);
    model->setData(model->index(row, 5), "pieces");
    model->setData(model->index(row, 6), 0.0);
    table->selectRow(row);
    table->edit(model->index(row, 1));
}

void ProductsPage::deleteProduct()
{
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete Product", "Please select a row to delete first.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete",
                               "Are you sure you want to delete the selected product(s)?")
        != QMessageBox::Yes) {
        return;
    }

    // Remove from the highest row index down, so indices don't shift under us.
    std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });
    for (const QModelIndex &index : selected)
        model->removeRow(index.row());

    saveChanges();
}

void ProductsPage::saveChanges()
{
    if (!model->submitAll()) {
        QMessageBox::warning(this, "Save Failed", model->lastError().text());
        model->revertAll();
    } else {
        model->select(); // Refresh so relational display names stay correct.
    }
}

void ProductsPage::revertChanges()
{
    model->revertAll();
}

void ProductsPage::searchProducts(const QString &text)
{
    if (text.isEmpty()) {
        model->setFilter("");
    } else {
        QString escaped = text;
        escaped.replace("'", "''"); // basic escaping for the LIKE filter
        model->setFilter(QString("ProductName LIKE '%%1%'").arg(escaped));
    }
    model->select();
}
