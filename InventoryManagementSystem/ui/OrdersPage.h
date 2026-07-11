#pragma once

#include <QWidget>

class QTableView;
class QSqlQueryModel;
class QComboBox;
class QPushButton;

// Restaurant Management > Customer Orders (staff side).
// Staff can view all incoming orders and move them through the
// six-stage status pipeline.
class OrdersPage : public QWidget
{
    Q_OBJECT

public:
    explicit OrdersPage(QWidget *parent = nullptr);

private slots:
    void refreshOrders();
    void updateSelectedOrderStatus();

private:
    QTableView *ordersTable;
    QSqlQueryModel *ordersModel;
    QComboBox *statusCombo;
    QPushButton *updateButton;
    QPushButton *refreshButton;
};
