/**
 * @file dlg_roll_dice.h
 * @ingroup GameDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_ROLL_DICE_H
#define DLG_ROLL_DICE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>

class DlgRollDice : public QDialog
{
    Q_OBJECT

    static constexpr uint DEFAULT_NUMBER_SIDES_DIE = 20;
    static constexpr uint DEFAULT_NUMBER_DICE_TO_ROLL = 1;

    QLabel *numberOfSidesLabel, *numberOfDiceLabel;
    QSpinBox *numberOfSidesEdit, *numberOfDiceEdit;
    QDialogButtonBox *buttonBox;

public:
    explicit DlgRollDice(QWidget *parent = nullptr);
    [[nodiscard]] uint getDieSideCount() const;
    [[nodiscard]] uint getDiceToRollCount() const;
};

#endif // DLG_ROLL_DICE_H
