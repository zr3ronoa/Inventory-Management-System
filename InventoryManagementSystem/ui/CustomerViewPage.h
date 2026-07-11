#pragma once

#include <QWidget>
#include <QMap>

class QLineEdit;
class QTableWidget;
class QPushButton;
class QLabel;
class QTableView;
class QSqlQueryModel;

// Customer Ordering module.
// - Lets a customer browse the menu and build a cart.
// - Places the order: this is where "Inventory Automation" happens
//   (checks stock, deducts ingredient quantities, all inside one transaction).
// - Lets the customer look up their past/active orders and see a simple
//   progress tracker for the selected order's status.
class CustomerViewPage : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerViewPage(QWidget *parent = nullptr);

private slots:
    void loadMenu();
    void addSelectedToCart();
    void removeSelectedCartRow();
    void placeOrder();
    void refreshMyOrders();
    void showProgressForSelectedOrder();

private:
    int findOrCreateCustomer(const QString &name, const QString &phone);
    double cartTotal() const;

    // --- Customer identity ---
    QLineEdit *nameEdit;
    QLineEdit *phoneEdit;

    // --- Menu browsing ---
    QTableWidget *menuTable; // columns: MenuID(hidden), Item Name, Price, Qty spinbox

    // --- Cart ---
    QTableWidget *cartTable; // columns: MenuID(hidden), Item Name, Qty, Subtotal
    QLabel *cartTotalLabel;
    QPushButton *placeOrderButton;

    // --- My Orders / tracking ---
    QTableView *ordersTable;
    QSqlQueryModel *ordersModel;
    QLabel *progressLabel;
};
