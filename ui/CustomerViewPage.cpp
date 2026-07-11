#include "CustomerViewPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QMessageBox>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDateTime>
#include <QFont>
#include <QSplitter>

static const QStringList ORDER_STATUSES = {
    "Order Received", "Kitchen Received", "Preparing",
    "Ready", "Out for Delivery", "Delivered"
};

CustomerViewPage::CustomerViewPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Customer View", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // ---- Customer identity ----
    QGroupBox *idBox = new QGroupBox("Your Details", this);
    QFormLayout *idForm = new QFormLayout(idBox);
    nameEdit = new QLineEdit(idBox);
    phoneEdit = new QLineEdit(idBox);
    idForm->addRow("Name:", nameEdit);
    idForm->addRow("Phone:", phoneEdit);
    mainLayout->addWidget(idBox);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // ---- Menu browsing (left) ----
    QWidget *menuWidget = new QWidget(splitter);
    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->addWidget(new QLabel("Menu", menuWidget));

    menuTable = new QTableWidget(0, 4, menuWidget);
    menuTable->setHorizontalHeaderLabels({"MenuID", "Item", "Price", "Qty"});
    menuTable->setColumnHidden(0, true);
    menuTable->horizontalHeader()->setStretchLastSection(true);
    menuTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    menuTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    menuLayout->addWidget(menuTable);

    QPushButton *addToCartButton = new QPushButton("Add to Cart", menuWidget);
    menuLayout->addWidget(addToCartButton);
    splitter->addWidget(menuWidget);

    // ---- Cart (right) ----
    QWidget *cartWidget = new QWidget(splitter);
    QVBoxLayout *cartLayout = new QVBoxLayout(cartWidget);
    cartLayout->setContentsMargins(0, 0, 0, 0);
    cartLayout->addWidget(new QLabel("Your Cart", cartWidget));

    cartTable = new QTableWidget(0, 4, cartWidget);
    cartTable->setHorizontalHeaderLabels({"MenuID", "Item", "Qty", "Subtotal"});
    cartTable->setColumnHidden(0, true);
    cartTable->horizontalHeader()->setStretchLastSection(true);
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartLayout->addWidget(cartTable);

    QHBoxLayout *cartButtons = new QHBoxLayout();
    QPushButton *removeFromCartButton = new QPushButton("Remove Selected", cartWidget);
    cartButtons->addWidget(removeFromCartButton);
    cartButtons->addStretch();
    cartLayout->addLayout(cartButtons);

    cartTotalLabel = new QLabel("Total: 0.00", cartWidget);
    QFont totalFont = cartTotalLabel->font();
    totalFont.setBold(true);
    cartTotalLabel->setFont(totalFont);
    cartLayout->addWidget(cartTotalLabel);

    placeOrderButton = new QPushButton("Place Order", cartWidget);
    cartLayout->addWidget(placeOrderButton);

    splitter->addWidget(cartWidget);
    mainLayout->addWidget(splitter, 1);

    // ---- My Orders / tracking ----
    QGroupBox *ordersBox = new QGroupBox("My Orders", this);
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersBox);

    QHBoxLayout *ordersToolbar = new QHBoxLayout();
    QPushButton *refreshOrdersButton = new QPushButton("Refresh My Orders", ordersBox);
    ordersToolbar->addWidget(refreshOrdersButton);
    ordersToolbar->addStretch();
    ordersLayout->addLayout(ordersToolbar);

    ordersModel = new QSqlQueryModel(this);
    ordersTable = new QTableView(ordersBox);
    ordersTable->setModel(ordersModel);
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setMaximumHeight(160);
    ordersLayout->addWidget(ordersTable);

    progressLabel = new QLabel("Select an order above to see its status.", ordersBox);
    progressLabel->setWordWrap(true);
    ordersLayout->addWidget(progressLabel);

    mainLayout->addWidget(ordersBox);

    // ---- connections ----
    connect(addToCartButton, &QPushButton::clicked, this, &CustomerViewPage::addSelectedToCart);
    connect(removeFromCartButton, &QPushButton::clicked, this, &CustomerViewPage::removeSelectedCartRow);
    connect(placeOrderButton, &QPushButton::clicked, this, &CustomerViewPage::placeOrder);
    connect(refreshOrdersButton, &QPushButton::clicked, this, &CustomerViewPage::refreshMyOrders);
    connect(ordersTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &CustomerViewPage::showProgressForSelectedOrder);

    loadMenu();
}

void CustomerViewPage::loadMenu()
{
    menuTable->setRowCount(0);
    QSqlQuery q("SELECT MenuID, ItemName, SellingPrice FROM MenuItems ORDER BY ItemName");
    int row = 0;
    while (q.next()) {
        menuTable->insertRow(row);
        menuTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        menuTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        menuTable->setItem(row, 2, new QTableWidgetItem(QString::number(q.value(2).toDouble(), 'f', 2)));

        QSpinBox *qtySpin = new QSpinBox(menuTable);
        qtySpin->setRange(1, 50);
        qtySpin->setValue(1);
        menuTable->setCellWidget(row, 3, qtySpin);

        row++;
    }
}

void CustomerViewPage::addSelectedToCart()
{
    int row = menuTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Add to Cart", "Please select a menu item first.");
        return;
    }

    QString menuId = menuTable->item(row, 0)->text();
    QString itemName = menuTable->item(row, 1)->text();
    double price = menuTable->item(row, 2)->text().toDouble();
    QSpinBox *qtySpin = qobject_cast<QSpinBox*>(menuTable->cellWidget(row, 3));
    int qty = qtySpin ? qtySpin->value() : 1;

    // If this item is already in the cart, just increase its quantity instead
    // of adding a duplicate row.
    for (int r = 0; r < cartTable->rowCount(); ++r) {
        if (cartTable->item(r, 0)->text() == menuId) {
            int existingQty = cartTable->item(r, 2)->text().toInt();
            int newQty = existingQty + qty;
            cartTable->item(r, 2)->setText(QString::number(newQty));
            cartTable->item(r, 3)->setText(QString::number(price * newQty, 'f', 2));
            cartTotalLabel->setText(QString("Total: %1").arg(QString::number(cartTotal(), 'f', 2)));
            return;
        }
    }

    int newRow = cartTable->rowCount();
    cartTable->insertRow(newRow);
    cartTable->setItem(newRow, 0, new QTableWidgetItem(menuId));
    cartTable->setItem(newRow, 1, new QTableWidgetItem(itemName));
    cartTable->setItem(newRow, 2, new QTableWidgetItem(QString::number(qty)));
    cartTable->setItem(newRow, 3, new QTableWidgetItem(QString::number(price * qty, 'f', 2)));

    cartTotalLabel->setText(QString("Total: %1").arg(QString::number(cartTotal(), 'f', 2)));
}

void CustomerViewPage::removeSelectedCartRow()
{
    int row = cartTable->currentRow();
    if (row < 0)
        return;
    cartTable->removeRow(row);
    cartTotalLabel->setText(QString("Total: %1").arg(QString::number(cartTotal(), 'f', 2)));
}

double CustomerViewPage::cartTotal() const
{
    double total = 0.0;
    for (int r = 0; r < cartTable->rowCount(); ++r)
        total += cartTable->item(r, 3)->text().toDouble();
    return total;
}

int CustomerViewPage::findOrCreateCustomer(const QString &name, const QString &phone)
{
    QSqlQuery find;
    find.prepare("SELECT CustomerID FROM Customers WHERE Name = ? AND Phone = ?");
    find.addBindValue(name);
    find.addBindValue(phone);
    find.exec();
    if (find.next())
        return find.value(0).toInt();

    QSqlQuery insert;
    insert.prepare("INSERT INTO Customers (Name, Phone) VALUES (?, ?)");
    insert.addBindValue(name);
    insert.addBindValue(phone);
    insert.exec();
    return insert.lastInsertId().toInt();
}

// ============================================================
// This is the "Inventory Automation" requirement:
// placing an order must (1) verify enough stock exists for every
// ingredient needed, and only if so, (2) save the order and
// (3) deduct the used ingredient quantities from Products.
// Everything happens inside a single database transaction so that
// the order and the stock deduction either both succeed or both fail.
// ============================================================
void CustomerViewPage::placeOrder()
{
    QString name = nameEdit->text().trimmed();
    QString phone = phoneEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Missing Name", "Please enter your name before placing an order.");
        return;
    }
    if (cartTable->rowCount() == 0) {
        QMessageBox::warning(this, "Empty Cart", "Your cart is empty. Add some items first.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    // Step 1: work out how much of each raw ingredient this order needs in total.
    QMap<int, double> requiredQty; // ProductID -> total quantity needed
    for (int r = 0; r < cartTable->rowCount(); ++r) {
        int menuId = cartTable->item(r, 0)->text().toInt();
        int qty = cartTable->item(r, 2)->text().toInt();

        QSqlQuery ing;
        ing.prepare("SELECT ProductID, QuantityRequired FROM IngredientUsage WHERE MenuID = ?");
        ing.addBindValue(menuId);
        ing.exec();
        while (ing.next()) {
            int productId = ing.value(0).toInt();
            double perUnit = ing.value(1).toDouble();
            requiredQty[productId] += perUnit * qty;
        }
    }

    // Step 2: verify there is enough stock for every ingredient needed.
    for (auto it = requiredQty.constBegin(); it != requiredQty.constEnd(); ++it) {
        QSqlQuery stockCheck;
        stockCheck.prepare("SELECT ProductName, QuantityInStock, Unit FROM Products WHERE ProductID = ?");
        stockCheck.addBindValue(it.key());
        stockCheck.exec();
        if (stockCheck.next()) {
            double available = stockCheck.value(1).toDouble();
            if (available < it.value()) {
                db.rollback();
                QMessageBox::critical(this, "Insufficient Stock",
                    QString("Not enough '%1' in stock.\nRequired: %2 %3, Available: %4 %3\n\n"
                            "Order was NOT placed.")
                        .arg(stockCheck.value(0).toString())
                        .arg(it.value(), 0, 'f', 2)
                        .arg(stockCheck.value(2).toString())
                        .arg(available, 0, 'f', 2));
                return;
            }
        }
    }

    // Step 3: everything checks out - find/create the customer and save the order.
    int customerId = findOrCreateCustomer(name, phone);

    QSqlQuery insertOrder;
    insertOrder.prepare("INSERT INTO Orders (CustomerID, Date, TotalPrice, Status) VALUES (?, ?, ?, ?)");
    insertOrder.addBindValue(customerId);
    insertOrder.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    insertOrder.addBindValue(cartTotal());
    insertOrder.addBindValue(ORDER_STATUSES.first()); // "Order Received"
    if (!insertOrder.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Order Failed", insertOrder.lastError().text());
        return;
    }
    int orderId = insertOrder.lastInsertId().toInt();

    for (int r = 0; r < cartTable->rowCount(); ++r) {
        int menuId = cartTable->item(r, 0)->text().toInt();
        int qty = cartTable->item(r, 2)->text().toInt();

        QSqlQuery insertItem;
        insertItem.prepare("INSERT INTO OrderItems (OrderID, MenuID, Quantity) VALUES (?, ?, ?)");
        insertItem.addBindValue(orderId);
        insertItem.addBindValue(menuId);
        insertItem.addBindValue(qty);
        insertItem.exec();
    }

    // Step 4: deduct the ingredients actually used from inventory.
    for (auto it = requiredQty.constBegin(); it != requiredQty.constEnd(); ++it) {
        QSqlQuery deduct;
        deduct.prepare("UPDATE Products SET QuantityInStock = QuantityInStock - ? WHERE ProductID = ?");
        deduct.addBindValue(it.value());
        deduct.addBindValue(it.key());
        deduct.exec();
    }

    db.commit();

    QMessageBox::information(this, "Order Placed",
        QString("Your order #%1 has been placed successfully!").arg(orderId));

    cartTable->setRowCount(0);
    cartTotalLabel->setText("Total: 0.00");
    refreshMyOrders();
}

void CustomerViewPage::refreshMyOrders()
{
    QString name = nameEdit->text().trimmed();
    QString phone = phoneEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::information(this, "My Orders", "Enter your name (and phone) above, then click Refresh.");
        return;
    }

    QSqlQuery q;
    q.prepare(
        "SELECT Orders.OrderID AS 'Order ID', Orders.Date AS 'Date', "
        "Orders.TotalPrice AS 'Total', Orders.Status AS 'Status' "
        "FROM Orders JOIN Customers ON Orders.CustomerID = Customers.CustomerID "
        "WHERE Customers.Name = ? AND Customers.Phone = ? "
        "ORDER BY Orders.OrderID DESC"
    );
    q.addBindValue(name);
    q.addBindValue(phone);
    q.exec();

    ordersModel->setQuery(std::move(q));
}

void CustomerViewPage::showProgressForSelectedOrder()
{
    QModelIndexList selected = ordersTable->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;

    QString status = ordersModel->data(ordersModel->index(selected.first().row(), 3)).toString();
    int currentIndex = ORDER_STATUSES.indexOf(status);

    QString progressText;
    for (int i = 0; i < ORDER_STATUSES.size(); ++i) {
        progressText += (i <= currentIndex) ? "\u2714 " : "\u25CB "; // check mark / circle
        progressText += ORDER_STATUSES[i];
        if (i != ORDER_STATUSES.size() - 1)
            progressText += "\n";
    }
    progressLabel->setText(progressText);
}
