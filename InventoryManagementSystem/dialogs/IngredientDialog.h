#pragma once

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

// A small dialog used by MenuPage to assign a Product (ingredient) and
// the quantity required to a MenuItem. This is what populates the
// IngredientUsage bridge table (the Many-to-Many relationship).
class IngredientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IngredientDialog(QWidget *parent = nullptr);

    int selectedProductId() const;
    double quantityRequired() const;

private:
    void loadProducts();

    QComboBox *productCombo;
    QDoubleSpinBox *quantitySpin;
};
