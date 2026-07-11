#pragma once

#include <QMainWindow>

class QListWidget;
class QStackedWidget;
class DashboardPage;
class ProductsPage;
class CategoriesPage;
class SuppliersPage;
class MenuPage;
class OrdersPage;
class CustomerViewPage;
class KDSPage;

// MainWindow provides the overall application shell:
//   - Menu bar (File > Exit)
//   - Left sidebar navigation (QListWidget)
//   - Main content area (QStackedWidget, one page per module)
//   - Status bar
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupMenuBar();
    void setupSidebar();
    void setupPages();
    void setupStatusBar();

    QListWidget *sidebar;
    QStackedWidget *stack;

    DashboardPage    *dashboardPage;
    ProductsPage     *productsPage;
    CategoriesPage   *categoriesPage;
    SuppliersPage    *suppliersPage;
    MenuPage         *menuPage;
    OrdersPage       *ordersPage;
    CustomerViewPage *customerViewPage;
    KDSPage          *kdsPage;
};
