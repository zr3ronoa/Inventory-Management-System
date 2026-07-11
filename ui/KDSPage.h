#pragma once

#include <QWidget>
#include <QDateTime>

class QLabel;
class QVBoxLayout;
class QScrollArea;
class QTimer;

// Kitchen Display System (KDS).
//
// A touch/board-friendly, always-on screen for the kitchen: every order
// that has reached the kitchen is shown as a ticket card in one of three
// columns - New, Preparing, Ready - mirroring the six-stage Orders.Status
// pipeline already used by OrdersPage/CustomerViewPage:
//
//   Order Received / Kitchen Received  -> "New"       column
//   Preparing                          -> "Preparing" column
//   Ready                              -> "Ready"      column
//   Out for Delivery / Delivered       -> leaves the KDS board
//
// Tapping the button on a ticket advances that order to the next stage.
// The board rebuilds itself on a timer so elapsed-time badges and new
// orders stay current without any manual action.
class KDSPage : public QWidget
{
    Q_OBJECT

public:
    explicit KDSPage(QWidget *parent = nullptr);

public slots:
    // Re-queries the database and rebuilds all three columns.
    // Safe to call anytime (e.g. when the user navigates to this page).
    void refresh();

private:
    struct OrderTicket {
        int orderId;
        QString customerName;
        QDateTime placedAt;
        QString status;
        double total;
    };

    QVBoxLayout *columnBody(const QString &key) const;
    void rebuildColumn(const QString &statusFilterSql, QVBoxLayout *body, QLabel *countLabel,
                        const QString &actionLabel, const QString &nextStatus);
    QWidget *buildTicketCard(const OrderTicket &ticket, const QString &actionLabel,
                              const QString &nextStatus);
    void clearLayout(QVBoxLayout *layout);
    void advanceOrder(int orderId, const QString &nextStatus);
    static QString elapsedText(const QDateTime &placedAt);
    static QString urgencyFor(const QDateTime &placedAt);

    QVBoxLayout *newColumnBody;
    QVBoxLayout *preparingColumnBody;
    QVBoxLayout *readyColumnBody;

    QLabel *newCountLabel;
    QLabel *preparingCountLabel;
    QLabel *readyCountLabel;

    QLabel *clockLabel;
    QTimer *refreshTimer; // rebuilds the whole board every few seconds
    QTimer *clockTimer;   // ticks the header clock every second
};
