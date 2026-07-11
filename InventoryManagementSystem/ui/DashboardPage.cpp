#include "DashboardPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableView>
#include <QFrame>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QFont>
#include <QHeaderView>
#include <QDateTime>
#include <QLocale>
#include <utility>

// Maps an order status to the pill color used across Dashboard + KDS,
// so the same status always reads the same color everywhere in the app.
static QString colorForStatus(const QString &status)
{
    if (status == "Order Received")   return "blue";
    if (status == "Kitchen Received") return "teal";
    if (status == "Preparing")        return "orange";
    if (status == "Ready")            return "green";
    if (status == "Out for Delivery") return "blue";
    if (status == "Delivered")        return "gray";
    return "gray";
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // ---- Header ----
    QVBoxLayout *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(2);
    QLabel *title = new QLabel("Dashboard", this);
    title->setObjectName("pageTitle");
    QLabel *subtitle = new QLabel("A live overview of inventory, menu and kitchen activity.", this);
    subtitle->setObjectName("pageSubtitle");
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    mainLayout->addLayout(headerLayout);

    // ---- KPI cards ----
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(14);
    cardsLayout->addWidget(buildStatCard(IconWidget::Kind::Box, "box.png",
                                          "Total Products", "teal", totalProductsLabel));
    cardsLayout->addWidget(buildStatCard(IconWidget::Kind::Menu, "menu.png",
                                          "Menu Items", "blue", totalMenuItemsLabel));
    cardsLayout->addWidget(buildStatCard(IconWidget::Kind::Receipt, "receipt.png",
                                          "Total Orders", "purple", totalOrdersLabel));
    cardsLayout->addWidget(buildStatCard(IconWidget::Kind::Clock, "clock.png",
                                          "Active in Kitchen", "orange", activeKitchenLabel));
    cardsLayout->addWidget(buildStatCard(IconWidget::Kind::Warning, "warning.png",
                                          "Low Stock Items", "red", lowStockLabel));
    mainLayout->addLayout(cardsLayout);

    // ---- Status breakdown strip ----
    QFrame *statusCard = new QFrame(this);
    statusCard->setObjectName("sectionCard");
    QVBoxLayout *statusCardLayout = new QVBoxLayout(statusCard);
    statusCardLayout->setContentsMargins(16, 12, 16, 14);

    QLabel *statusHeading = new QLabel("Orders by Status", statusCard);
    statusHeading->setObjectName("sectionHeading");
    statusCardLayout->addWidget(statusHeading);

    statusStripLayout = new QHBoxLayout();
    statusStripLayout->setSpacing(10);
    statusCardLayout->addLayout(statusStripLayout);

    mainLayout->addWidget(statusCard);

    // ---- Middle row: low stock alerts (left) + recent orders (right) ----
    QHBoxLayout *middleRow = new QHBoxLayout();
    middleRow->setSpacing(14);

    // Low stock alert panel
    QFrame *lowStockCard = new QFrame(this);
    lowStockCard->setObjectName("sectionCard");
    lowStockCard->setMinimumWidth(280);
    lowStockCard->setMaximumWidth(320);
    QVBoxLayout *lowStockCardLayout = new QVBoxLayout(lowStockCard);
    lowStockCardLayout->setContentsMargins(16, 12, 16, 14);

    QLabel *lowStockHeading = new QLabel("Low Stock Alerts", lowStockCard);
    lowStockHeading->setObjectName("sectionHeading");
    IconWidget *lowStockIcon = new IconWidget(IconWidget::Kind::Warning, "warning.png", lowStockCard);
    lowStockIcon->setFixedSize(20, 20);
    QHBoxLayout *lowStockHeadingRow = new QHBoxLayout();
    lowStockHeadingRow->setSpacing(8);
    lowStockHeadingRow->addWidget(lowStockIcon);
    lowStockHeadingRow->addWidget(lowStockHeading);
    lowStockHeadingRow->addStretch();
    lowStockCardLayout->addLayout(lowStockHeadingRow);

    lowStockListLayout = new QVBoxLayout();
    lowStockListLayout->setSpacing(6);
    lowStockCardLayout->addLayout(lowStockListLayout);

    lowStockEmptyHint = new QLabel("All stock levels look healthy.", lowStockCard);
    lowStockEmptyHint->setObjectName("ticketMeta");
    lowStockEmptyHint->setWordWrap(true);
    lowStockCardLayout->addWidget(lowStockEmptyHint);
    lowStockCardLayout->addStretch();

    middleRow->addWidget(lowStockCard);

    // Recent orders panel
    QFrame *recentCard = new QFrame(this);
    recentCard->setObjectName("sectionCard");
    QVBoxLayout *recentCardLayout = new QVBoxLayout(recentCard);
    recentCardLayout->setContentsMargins(16, 12, 16, 14);

    QLabel *recentLabel = new QLabel("Recent Orders", recentCard);
    recentLabel->setObjectName("sectionHeading");
    recentCardLayout->addWidget(recentLabel);

    recentOrdersTable = new QTableView(recentCard);
    recentOrdersModel = new QSqlQueryModel(this);
    recentOrdersTable->setModel(recentOrdersModel);
    recentOrdersTable->horizontalHeader()->setStretchLastSection(true);
    recentOrdersTable->verticalHeader()->setVisible(false);
    recentOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentOrdersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentOrdersTable->setAlternatingRowColors(true);
    recentCardLayout->addWidget(recentOrdersTable);

    middleRow->addWidget(recentCard, 1);

    mainLayout->addLayout(middleRow, 1);

    refresh();
}

QFrame *DashboardPage::buildStatCard(IconWidget::Kind iconKind, const QString &iconFile,
                                      const QString &title, const QString &accent, QLabel *&valueLabelOut)
{
    QFrame *card = new QFrame(this);
    card->setObjectName("statCard");
    card->setProperty("accent", accent);
    card->setMinimumHeight(96);

    QHBoxLayout *outer = new QHBoxLayout(card);
    outer->setContentsMargins(16, 12, 16, 12);

    IconWidget *icon = new IconWidget(iconKind, iconFile, card);
    icon->setFixedSize(32, 32);
    outer->addWidget(icon);

    QVBoxLayout *textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    valueLabelOut = new QLabel("0", card);
    valueLabelOut->setObjectName("statValue");
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("statTitle");
    textCol->addWidget(valueLabelOut);
    textCol->addWidget(titleLabel);

    outer->addLayout(textCol);
    outer->addStretch();

    return card;
}

QLabel *DashboardPage::buildPill(const QString &text, const QString &colorKey)
{
    QLabel *pill = new QLabel(text);
    pill->setProperty("pill", true);
    pill->setProperty("pillColor", colorKey);
    return pill;
}

void DashboardPage::refresh()
{
    QSqlQuery q;

    q.exec("SELECT COUNT(*) FROM Products");
    q.next();
    totalProductsLabel->setText(q.value(0).toString());

    q.exec("SELECT COUNT(*) FROM MenuItems");
    q.next();
    totalMenuItemsLabel->setText(q.value(0).toString());

    q.exec("SELECT COUNT(*) FROM Orders");
    q.next();
    totalOrdersLabel->setText(q.value(0).toString());

    // "Active in kitchen" = orders that have reached the kitchen but
    // haven't left it yet - exactly the orders the KDS page tracks.
    q.exec("SELECT COUNT(*) FROM Orders WHERE Status IN "
           "('Kitchen Received', 'Preparing', 'Ready')");
    q.next();
    activeKitchenLabel->setText(q.value(0).toString());

    q.prepare("SELECT COUNT(*) FROM Products WHERE QuantityInStock < ?");
    q.addBindValue(LOW_STOCK_THRESHOLD);
    q.exec();
    q.next();
    lowStockLabel->setText(q.value(0).toString());

    // ---- Status breakdown pills ----
    // Clear every item in the strip (pills *and* the trailing stretch from
    // the previous refresh), not just the widgets we're tracking, so the
    // layout doesn't accumulate an extra stretch on every refresh() call.
    {
        QLayoutItem *stripChild;
        while ((stripChild = statusStripLayout->takeAt(0)) != nullptr) {
            if (stripChild->widget())
                stripChild->widget()->deleteLater();
            delete stripChild;
        }
    }
    statusPillWidgets.clear();

    q.exec("SELECT Status, COUNT(*) FROM Orders GROUP BY Status");
    bool anyStatus = false;
    while (q.next()) {
        anyStatus = true;
        QString status = q.value(0).toString();
        int count = q.value(1).toInt();
        QLabel *pill = buildPill(QString("%1  \u00B7  %2").arg(status).arg(count),
                                  colorForStatus(status));
        statusStripLayout->addWidget(pill);
        statusPillWidgets.append(pill);
    }
    if (!anyStatus) {
        QLabel *pill = buildPill("No orders yet", "gray");
        statusStripLayout->addWidget(pill);
        statusPillWidgets.append(pill);
    }
    statusStripLayout->addStretch();

    // ---- Low stock alert list ----
    QLayoutItem *child;
    while ((child = lowStockListLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    q.prepare("SELECT ProductName, QuantityInStock, Unit FROM Products "
              "WHERE QuantityInStock < ? ORDER BY QuantityInStock ASC");
    q.addBindValue(LOW_STOCK_THRESHOLD);
    q.exec();

    bool anyLowStock = false;
    while (q.next()) {
        anyLowStock = true;
        QString line = QString("%1  \u2014  %2 %3 left")
                            .arg(q.value(0).toString())
                            .arg(QLocale().toString(q.value(1).toDouble(), 'f', 1))
                            .arg(q.value(2).toString());
        QLabel *rowLabel = new QLabel(line, this);
        rowLabel->setObjectName("ticketItem");
        rowLabel->setStyleSheet("color:#B91C1C; font-weight:600;");
        rowLabel->setWordWrap(true);
        lowStockListLayout->addWidget(rowLabel);
    }
    lowStockEmptyHint->setVisible(!anyLowStock);

    // ---- Recent orders table ----
    recentOrdersModel->setQuery(
        "SELECT Orders.OrderID AS 'Order ID', Customers.Name AS 'Customer', "
        "Orders.Date AS 'Date', Orders.TotalPrice AS 'Total', Orders.Status AS 'Status' "
        "FROM Orders "
        "JOIN Customers ON Orders.CustomerID = Customers.CustomerID "
        "ORDER BY Orders.OrderID DESC LIMIT 10"
    );
}
