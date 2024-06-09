#include "dlg_roll_dice.h"

#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <server_player.h>

DlgRollDice::DlgRollDice(QWidget *parent) : QDialog(parent)
{
    numberOfSidesLabel = new QLabel(tr("Number of sides:"));
    numberOfSidesEdit = new QLineEdit(QString::number(DEFAULT_NUMBER_SIDES_DIE));
    numberOfSidesEdit->setValidator(new QIntValidator(Server_Player::MINIMUM_DIE_SIDES, Server_Player::MAXIMUM_DIE_SIDES, numberOfSidesEdit));
    numberOfSidesLabel->setBuddy(numberOfSidesEdit);

    numberOfDiceLabel = new QLabel(tr("Number of dice:"));
    numberOfDiceEdit = new QLineEdit(QString::number(DEFAULT_NUMBER_DICE_TO_ROLL));
    numberOfDiceEdit->setValidator(new QIntValidator(Server_Player::MINIMUM_DICE_TO_ROLL, Server_Player::MAXIMUM_DICE_TO_ROLL, numberOfDiceEdit));
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
