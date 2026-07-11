#include "MainWindow.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QWidget>
#include <QApplication>

#include "DashboardPage.h"
#include "ProductsPage.h"
#include "CategoriesPage.h"
#include "SuppliersPage.h"
#include "MenuPage.h"
#include "OrdersPage.h"
#include "CustomerViewPage.h"
#include "KDSPage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Restaurant Inventory Management System");

    setupMenuBar();

    // Central widget: sidebar (left) + stacked pages (right)
    QWidget *central = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    sidebar = new QListWidget(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);
    sidebar->setSpacing(2);

    stack = new QStackedWidget(central);

    layout->addWidget(sidebar);
    layout->addWidget(stack, 1);

    setCentralWidget(central);

    setupPages();
    setupSidebar();
    setupStatusBar();

    connect(sidebar, &QListWidget::currentRowChanged, stack, &QStackedWidget::setCurrentIndex);
    sidebar->setCurrentRow(0);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Restaurant Inventory Management System - DBMS Course Project", 4000);
    });
}

void MainWindow::setupPages()
{
    dashboardPage    = new DashboardPage(this);
    productsPage     = new ProductsPage(this);
    categoriesPage   = new CategoriesPage(this);
    suppliersPage    = new SuppliersPage(this);
    menuPage         = new MenuPage(this);
    ordersPage       = new OrdersPage(this);
    customerViewPage = new CustomerViewPage(this);
    kdsPage          = new KDSPage(this);

    stack->addWidget(dashboardPage);    // index 0
    stack->addWidget(productsPage);     // index 1
    stack->addWidget(categoriesPage);   // index 2
    stack->addWidget(suppliersPage);    // index 3
    stack->addWidget(menuPage);         // index 4
    stack->addWidget(ordersPage);       // index 5
    stack->addWidget(customerViewPage); // index 6
    stack->addWidget(kdsPage);          // index 7

    // Refresh the dashboard / KDS every time we switch to them, so they
    // always reflect the latest data immediately (their own timers keep
    // them current while they stay on screen too).
    connect(stack, &QStackedWidget::currentChanged, this, [this](int index) {
        if (stack->widget(index) == dashboardPage)
            dashboardPage->refresh();
        else if (stack->widget(index) == kdsPage)
            kdsPage->refresh();
    });
}

void MainWindow::setupSidebar()
{
    sidebar->addItem("Dashboard");
    sidebar->addItem("Inventory - Products");
    sidebar->addItem("Inventory - Categories");
    sidebar->addItem("Inventory - Suppliers");
    sidebar->addItem("Menu Management");
    sidebar->addItem("Customer Orders (Staff)");
    sidebar->addItem("Customer View");
    sidebar->addItem("Kitchen Display (KDS)");
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Connected to restaurant.db");
}
