#ifndef DLG_ROLL_DICE_H
#define DLG_ROLL_DICE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>

class DlgRollDice : public QDialog
{
    Q_OBJECT

    const int DEFAULT_NUMBER_SIDES_DIE = 20;
    const int MINIMUM_DIE_SIDES = 2;
    const int MAXIMUM_DIE_SIDES = 1000000;
    const int DEFAULT_NUMBER_DICE_TO_ROLL = 1;
    const int MINIMUM_DICE_TO_ROLL = 1;
    const int MAXIMUM_DICE_TO_ROLL = 100;

    QLabel *numberOfSidesLabel, *numberOfDiceLabel;
    QLineEdit *numberOfSidesEdit, *numberOfDiceEdit;
    QDialogButtonBox *buttonBox;

public:
    explicit DlgRollDice(QWidget *parent = nullptr);
    [[nodiscard]] uint getDieSideCount() const;
    [[nodiscard]] uint getDiceToRollCount() const;
};

#endif // DLG_ROLL_DICE_H
