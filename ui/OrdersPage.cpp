#include "OrdersPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QFont>

// The six statuses an order moves through, in order.
static const QStringList ORDER_STATUSES = {
    "Order Received", "Kitchen Received", "Preparing",
    "Ready", "Out for Delivery", "Delivered"
};

OrdersPage::OrdersPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Customer Orders (Staff View)", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    ordersModel = new QSqlQueryModel(this);
    ordersTable = new QTableView(this);
    ordersTable->setModel(ordersModel);
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setWordWrap(true);
    mainLayout->addWidget(ordersTable, 1);

    QHBoxLayout *actionRow = new QHBoxLayout();
    actionRow->addWidget(new QLabel("Set status of selected order to:", this));
    statusCombo = new QComboBox(this);
    statusCombo->addItems(ORDER_STATUSES);
    updateButton = new QPushButton("Update Status", this);
    refreshButton = new QPushButton("Refresh", this);
    actionRow->addWidget(statusCombo);
    actionRow->addWidget(updateButton);
    actionRow->addStretch();
    actionRow->addWidget(refreshButton);
    mainLayout->addLayout(actionRow);

    connect(updateButton, &QPushButton::clicked, this, &OrdersPage::updateSelectedOrderStatus);
    connect(refreshButton, &QPushButton::clicked, this, &OrdersPage::refreshOrders);

    refreshOrders();
}

void OrdersPage::refreshOrders()
{
    // Staff care about *what* to cook/serve, not an arbitrary order
    // number, so the first visible column is now a plain-language list
    // of items (e.g. "Chicken Burger x2, French Fries x1") instead of
    // Order ID. OrderID is still selected (as a hidden column) since
    // updateSelectedOrderStatus() needs it to know which order to update.
    ordersModel->setQuery(
        "SELECT Orders.OrderID AS 'OrderID', "
        "(SELECT GROUP_CONCAT(MenuItems.ItemName || ' x' || OrderItems.Quantity, ', ') "
        " FROM OrderItems JOIN MenuItems ON OrderItems.MenuID = MenuItems.MenuID "
        " WHERE OrderItems.OrderID = Orders.OrderID) AS 'Items', "
        "Customers.Name AS 'Customer', Customers.Phone AS 'Phone', "
        "Orders.Date AS 'Date', Orders.TotalPrice AS 'Total', Orders.Status AS 'Status' "
        "FROM Orders "
        "JOIN Customers ON Orders.CustomerID = Customers.CustomerID "
        "ORDER BY Orders.OrderID DESC"
    );

    ordersTable->setColumnHidden(0, true); // OrderID: kept for lookups, not shown
    ordersTable->setColumnWidth(1, 320);   // Items: give it room since it's the important column
    ordersTable->resizeRowsToContents();
}

void OrdersPage::updateSelectedOrderStatus()
{
    QModelIndexList selected = ordersTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Update Status", "Please select an order first.");
        return;
    }

    int orderId = ordersModel->data(ordersModel->index(selected.first().row(), 0)).toInt();
    QString newStatus = statusCombo->currentText();

    QSqlQuery q;
    q.prepare("UPDATE Orders SET Status = ? WHERE OrderID = ?");
    q.addBindValue(newStatus);
    q.addBindValue(orderId);

    if (!q.exec()) {
        QMessageBox::warning(this, "Update Failed", q.lastError().text());
        return;
    }

    refreshOrders();
}
