#include "dlg_roll_dice.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/utility/trice_limits.h>

DlgRollDice::DlgRollDice(QWidget *parent) : QDialog(parent)
{
    numberOfSidesLabel = new QLabel(tr("Number of sides:"));
    numberOfSidesEdit = new QSpinBox(this);
    numberOfSidesEdit->setValue(DEFAULT_NUMBER_SIDES_DIE);
    numberOfSidesEdit->setRange(MINIMUM_DIE_SIDES, MAXIMUM_DIE_SIDES);
    numberOfSidesEdit->setFocus();
    numberOfSidesLabel->setBuddy(numberOfSidesEdit);

    numberOfDiceLabel = new QLabel(tr("Number of dice:"));
    numberOfDiceEdit = new QSpinBox(this);
    numberOfDiceEdit->setValue(DEFAULT_NUMBER_DICE_TO_ROLL);
    numberOfDiceEdit->setRange(MINIMUM_DICE_TO_ROLL, MAXIMUM_DICE_TO_ROLL);
    numberOfDiceLabel->setBuddy(numberOfDiceEdit);

    auto *grid = new QGridLayout;
    grid->addWidget(numberOfSidesLabel, 0, 0);
    grid->addWidget(numberOfSidesEdit, 0, 1);
    grid->addWidget(numberOfDiceLabel, 1, 0);
    grid->addWidget(numberOfDiceEdit, 1, 1);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addItem(grid);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Roll Dice"));
}

uint DlgRollDice::getDieSideCount() const
{
    return numberOfSidesEdit->text().toUInt();
}

uint DlgRollDice::getDiceToRollCount() const
{
    return numberOfDiceEdit->text().toUInt();
}
