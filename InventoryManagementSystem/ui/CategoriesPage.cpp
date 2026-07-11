#include "CategoriesPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlError>
#include <QHeaderView>
#include <QFont>
#include <algorithm>

CategoriesPage::CategoriesPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Inventory - Categories", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    QHBoxLayout *toolbar = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Add Category", this);
    QPushButton *deleteButton = new QPushButton("Delete Selected", this);
    QPushButton *saveButton = new QPushButton("Save Changes", this);
    QPushButton *revertButton = new QPushButton("Revert Changes", this);
    toolbar->addWidget(addButton);
    toolbar->addWidget(deleteButton);
    toolbar->addStretch();
    toolbar->addWidget(saveButton);
    toolbar->addWidget(revertButton);
    mainLayout->addLayout(toolbar);

    model = new QSqlTableModel(this);
    model->setTable("Categories");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(1, Qt::Horizontal, "Category Name");

    table = new QTableView(this);
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setColumnHidden(0, true);
    mainLayout->addWidget(table, 1);

    connect(addButton, &QPushButton::clicked, this, &CategoriesPage::addCategory);
    connect(deleteButton, &QPushButton::clicked, this, &CategoriesPage::deleteCategory);
    connect(saveButton, &QPushButton::clicked, this, &CategoriesPage::saveChanges);
    connect(revertButton, &QPushButton::clicked, this, &CategoriesPage::revertChanges);
}

void CategoriesPage::addCategory()
{
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 1), "New Category");
    table->selectRow(row);
    table->edit(model->index(row, 1));
}

void CategoriesPage::deleteCategory()
{
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete Category", "Please select a row to delete first.");
        return;
    }
    if (QMessageBox::question(this, "Confirm Delete", "Delete the selected category?") != QMessageBox::Yes)
        return;

    std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });
    for (const QModelIndex &index : selected)
        model->removeRow(index.row());
    saveChanges();
}

void CategoriesPage::saveChanges()
{
    if (!model->submitAll()) {
        QMessageBox::warning(this, "Save Failed", model->lastError().text());
        model->revertAll();
    } else {
        model->select();
    }
}

void CategoriesPage::revertChanges()
{
    model->revertAll();
}
