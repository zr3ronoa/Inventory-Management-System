#pragma once

#include <QWidget>
#include <QVector>
#include "IconWidget.h"

class QLabel;
class QTableView;
class QSqlQueryModel;
class QHBoxLayout;
class QVBoxLayout;
class QFrame;

// Shows an attractive, at-a-glance overview of the whole system: KPI cards,
// a live order-status breakdown, a low-stock alert panel, and a table of
// the most recent orders.
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

    // Re-runs all the summary queries and updates the cards/labels/table.
    // Called whenever the user navigates to this page.
    void refresh();

private:
    // ---- builders ----
    QFrame *buildStatCard(IconWidget::Kind iconKind, const QString &iconFile,
                           const QString &title, const QString &accent, QLabel *&valueLabelOut);
    QLabel *buildPill(const QString &text, const QString &colorKey);

    // ---- KPI cards ----
    QLabel *totalProductsLabel;
    QLabel *totalMenuItemsLabel;
    QLabel *totalOrdersLabel;
    QLabel *lowStockLabel;
    QLabel *activeKitchenLabel;

    // ---- status breakdown strip ----
    QHBoxLayout *statusStripLayout;
    QVector<QWidget*> statusPillWidgets;

    // ---- low stock alert panel ----
    QVBoxLayout *lowStockListLayout;
    QLabel *lowStockEmptyHint;

    // ---- recent orders ----
    QTableView *recentOrdersTable;
    QSqlQueryModel *recentOrdersModel;

    static const int LOW_STOCK_THRESHOLD = 5; // Below this quantity => "low stock"
};
