#include "KDSPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFont>
#include <QMessageBox>

// ---- column definitions: which statuses live in which column, and what
// action moves a ticket out of that column ----
namespace {
    const QString NEW_STATUSES       = "('Order Received', 'Kitchen Received')";
    const QString PREPARING_STATUSES = "('Preparing')";
    const QString READY_STATUSES     = "('Ready')";
}

KDSPage::KDSPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // ---- Header ----
    QHBoxLayout *headerRow = new QHBoxLayout();
    QVBoxLayout *headerText = new QVBoxLayout();
    headerText->setSpacing(2);
    QLabel *title = new QLabel("Kitchen Display System", this);
    title->setObjectName("pageTitle");
    QLabel *subtitle = new QLabel("Live kitchen ticket board \u2014 updates automatically.", this);
    subtitle->setObjectName("pageSubtitle");
    headerText->addWidget(title);
    headerText->addWidget(subtitle);
    headerRow->addLayout(headerText);
    headerRow->addStretch();

    clockLabel = new QLabel(this);
    clockLabel->setObjectName("kdsClock");
    headerRow->addWidget(clockLabel);

    QPushButton *refreshButton = new QPushButton("Refresh Now", this);
    refreshButton->setProperty("flat", true);
    connect(refreshButton, &QPushButton::clicked, this, &KDSPage::refresh);
    headerRow->addWidget(refreshButton);

    mainLayout->addLayout(headerRow);

    // ---- Board: 3 columns ----
    QHBoxLayout *board = new QHBoxLayout();
    board->setSpacing(14);

    auto buildColumn = [this](const QString &titleText, const QString &colColor,
                              QLabel *&countLabelOut, QVBoxLayout *&bodyOut) -> QWidget* {
        QFrame *column = new QFrame(this);
        column->setObjectName("kdsColumn");
        QVBoxLayout *colLayout = new QVBoxLayout(column);
        colLayout->setContentsMargins(0, 0, 0, 10);
        colLayout->setSpacing(8);

        QFrame *header = new QFrame(column);
        header->setObjectName("kdsColumnHeader");
        header->setProperty("colColor", colColor);
        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(14, 10, 14, 10);
        QLabel *colTitle = new QLabel(titleText, header);
        colTitle->setObjectName("kdsColumnTitle");
        countLabelOut = new QLabel("0", header);
        countLabelOut->setProperty("pill", true);
        countLabelOut->setStyleSheet("background-color: rgba(255,255,255,60); color:#FFFFFF;");
        headerLayout->addWidget(colTitle);
        headerLayout->addStretch();
        headerLayout->addWidget(countLabelOut);
        colLayout->addWidget(header);

        QScrollArea *scroll = new QScrollArea(column);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *scrollContent = new QWidget(scroll);
        bodyOut = new QVBoxLayout(scrollContent);
        bodyOut->setContentsMargins(10, 4, 10, 4);
        bodyOut->setSpacing(10);
        bodyOut->addStretch();
        scroll->setWidget(scrollContent);

        colLayout->addWidget(scroll, 1);
        return column;
    };

    board->addWidget(buildColumn("New Orders", "blue", newCountLabel, newColumnBody), 1);
    board->addWidget(buildColumn("Preparing", "orange", preparingCountLabel, preparingColumnBody), 1);
    board->addWidget(buildColumn("Ready for Pickup", "green", readyCountLabel, readyColumnBody), 1);

    mainLayout->addLayout(board, 1);

    // ---- Timers: rebuild the board every 4s, tick the clock every 1s ----
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &KDSPage::refresh);
    refreshTimer->start(4000);

    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, [this]() {
        clockLabel->setText(QDateTime::currentDateTime().toString("ddd, hh:mm:ss AP"));
    });
    clockTimer->start(1000);
    clockLabel->setText(QDateTime::currentDateTime().toString("ddd, hh:mm:ss AP"));

    refresh();
}

void KDSPage::clearLayout(QVBoxLayout *layout)
{
    // Remove every item except leave layout ready to re-add a trailing stretch.
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

QString KDSPage::elapsedText(const QDateTime &placedAt)
{
    qint64 secs = placedAt.secsTo(QDateTime::currentDateTime());
    if (secs < 0) secs = 0;
    qint64 mins = secs / 60;
    if (mins < 60)
        return QString("%1m ago").arg(mins);
    qint64 hours = mins / 60;
    return QString("%1h %2m ago").arg(hours).arg(mins % 60);
}

QString KDSPage::urgencyFor(const QDateTime &placedAt)
{
    qint64 mins = placedAt.secsTo(QDateTime::currentDateTime()) / 60;
    if (mins < 5)  return "ok";
    if (mins < 15) return "warn";
    return "late";
}

QWidget *KDSPage::buildTicketCard(const OrderTicket &ticket, const QString &actionLabel,
                                   const QString &nextStatus)
{
    QFrame *card = new QFrame();
    card->setObjectName("ticketCard");
    card->setProperty("urgency", urgencyFor(ticket.placedAt));

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(6);

    // Top row: order # + elapsed pill
    QHBoxLayout *topRow = new QHBoxLayout();
    QLabel *idLabel = new QLabel(QString("#%1").arg(ticket.orderId), card);
    idLabel->setObjectName("ticketOrderId");
    topRow->addWidget(idLabel);
    topRow->addStretch();

    QString urgency = urgencyFor(ticket.placedAt);
    QString pillColor = (urgency == "late") ? "red" : (urgency == "warn") ? "orange" : "green";
    QLabel *elapsedPill = new QLabel(elapsedText(ticket.placedAt), card);
    elapsedPill->setProperty("pill", true);
    elapsedPill->setProperty("pillColor", pillColor);
    topRow->addWidget(elapsedPill);
    layout->addLayout(topRow);

    // Meta row: customer + placed time
    QLabel *metaLabel = new QLabel(
        QString("%1  \u00B7  %2").arg(ticket.customerName, ticket.placedAt.toString("hh:mm AP")), card);
    metaLabel->setObjectName("ticketMeta");
    layout->addWidget(metaLabel);

    // Divider
    QFrame *divider = new QFrame(card);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color:#F1F5F9; max-height:1px; border:none;");
    layout->addWidget(divider);

    // Item lines
    QSqlQuery itemsQuery;
    itemsQuery.prepare(
        "SELECT MenuItems.ItemName, OrderItems.Quantity FROM OrderItems "
        "JOIN MenuItems ON OrderItems.MenuID = MenuItems.MenuID "
        "WHERE OrderItems.OrderID = ?");
    itemsQuery.addBindValue(ticket.orderId);
    itemsQuery.exec();
    while (itemsQuery.next()) {
        QLabel *itemLabel = new QLabel(
            QString("<b>%1x</b>  %2").arg(itemsQuery.value(1).toInt()).arg(itemsQuery.value(0).toString()),
            card);
        itemLabel->setObjectName("ticketItem");
        itemLabel->setWordWrap(true);
        layout->addWidget(itemLabel);
    }

    // Action button
    QPushButton *actionButton = new QPushButton(actionLabel, card);
    int orderId = ticket.orderId;
    connect(actionButton, &QPushButton::clicked, this, [this, orderId, nextStatus]() {
        advanceOrder(orderId, nextStatus);
    });
    layout->addWidget(actionButton);

    return card;
}

void KDSPage::rebuildColumn(const QString &statusFilterSql, QVBoxLayout *body, QLabel *countLabel,
                             const QString &actionLabel, const QString &nextStatus)
{
    clearLayout(body);

    QSqlQuery q;
    q.exec(QString(
        "SELECT Orders.OrderID, Customers.Name, Orders.Date, Orders.Status, Orders.TotalPrice "
        "FROM Orders JOIN Customers ON Orders.CustomerID = Customers.CustomerID "
        "WHERE Orders.Status IN %1 "
        "ORDER BY Orders.OrderID ASC").arg(statusFilterSql));

    int count = 0;
    while (q.next()) {
        OrderTicket ticket;
        ticket.orderId = q.value(0).toInt();
        ticket.customerName = q.value(1).toString();
        ticket.placedAt = QDateTime::fromString(q.value(2).toString(), "yyyy-MM-dd HH:mm:ss");
        ticket.status = q.value(3).toString();
        ticket.total = q.value(4).toDouble();

        body->addWidget(buildTicketCard(ticket, actionLabel, nextStatus));
        count++;
    }

    if (count == 0) {
        QLabel *emptyHint = new QLabel("No orders here right now.");
        emptyHint->setObjectName("emptyColumnHint");
        emptyHint->setAlignment(Qt::AlignCenter);
        body->addWidget(emptyHint);
    }

    // Trailing stretch keeps cards pinned to the top of the column instead
    // of spreading out to fill the scroll area.
    body->addStretch();

    countLabel->setText(QString::number(count));
}

void KDSPage::advanceOrder(int orderId, const QString &nextStatus)
{
    QSqlQuery q;
    q.prepare("UPDATE Orders SET Status = ? WHERE OrderID = ?");
    q.addBindValue(nextStatus);
    q.addBindValue(orderId);

    if (!q.exec()) {
        QMessageBox::warning(this, "Update Failed", q.lastError().text());
        return;
    }

    refresh();
}

void KDSPage::refresh()
{
    rebuildColumn(NEW_STATUSES, newColumnBody, newCountLabel,
                  "Start Preparing", "Preparing");
    rebuildColumn(PREPARING_STATUSES, preparingColumnBody, preparingCountLabel,
                  "Mark Ready", "Ready");
    rebuildColumn(READY_STATUSES, readyColumnBody, readyCountLabel,
                  "Picked Up / Out for Delivery", "Out for Delivery");
}
