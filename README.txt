Restaurant Inventory Management System
=======================================
Qt 6 (Widgets) + C++17 + SQLite (Qt SQL Module) + CMake

See SETUP_AND_RUN_GUIDE.txt (one folder up, next to this zip) for full
step-by-step instructions on installing prerequisites, building, and running
the project.

Quick summary of the folders:
  database/   - DatabaseManager: opens SQLite, creates tables, seeds sample data
  ui/         - MainWindow + one page per module (Dashboard, Products, Categories,
                Suppliers, Menu Management, Customer Orders (staff), Customer View)
  dialogs/    - IngredientDialog, used to assign a Product + quantity to a MenuItem
  resources/  - reserved for icons/images if you want to add them later

The SQLite database file (restaurant.db) is created automatically the first
time you run the app, in the same folder as the executable, and is pre-filled
with a few sample categories, suppliers, products, and menu items so the app
is immediately demoable.
