cmake_minimum_required(VERSION 3.16)
project(InventoryManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt-specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(SOURCES
    main.cpp
    database/DatabaseManager.cpp
    ui/MainWindow.cpp
    ui/DashboardPage.cpp
    ui/ProductsPage.cpp
    ui/CategoriesPage.cpp
    ui/SuppliersPage.cpp
    ui/MenuPage.cpp
    ui/OrdersPage.cpp
    ui/CustomerViewPage.cpp
    ui/KDSPage.cpp
    ui/IconWidget.cpp
    dialogs/IngredientDialog.cpp
)

set(HEADERS
    database/DatabaseManager.h
    ui/MainWindow.h
    ui/DashboardPage.h
    ui/ProductsPage.h
    ui/CategoriesPage.h
    ui/SuppliersPage.h
    ui/MenuPage.h
    ui/OrdersPage.h
    ui/CustomerViewPage.h
    ui/KDSPage.h
    ui/IconWidget.h
    dialogs/IngredientDialog.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(InventoryManagementSystem ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(InventoryManagementSystem PRIVATE Qt6::Widgets Qt6::Sql)

target_include_directories(InventoryManagementSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
)
