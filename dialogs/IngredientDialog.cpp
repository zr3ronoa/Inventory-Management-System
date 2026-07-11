#include "IngredientDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QSqlQuery>

IngredientDialog::IngredientDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Assign Ingredient");
    setMinimumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    productCombo = new QComboBox(this);
    loadProducts();

    quantitySpin = new QDoubleSpinBox(this);
    quantitySpin->setRange(0.01, 100000);
    quantitySpin->setDecimals(2);
    quantitySpin->setValue(1.0);

    form->addRow("Product (Ingredient):", productCombo);
    form->addRow("Quantity Required:", quantitySpin);
    mainLayout->addLayout(form);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

void IngredientDialog::loadProducts()
{
    QSqlQuery q("SELECT ProductID, ProductName, Unit FROM Products ORDER BY ProductName");
    while (q.next()) {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString unit = q.value(2).toString();
        productCombo->addItem(QString("%1 (%2)").arg(name, unit), id);
    }
}

int IngredientDialog::selectedProductId() const
{
    return productCombo->currentData().toInt();
}

double IngredientDialog::quantityRequired() const
{
    return quantitySpin->value();
}
