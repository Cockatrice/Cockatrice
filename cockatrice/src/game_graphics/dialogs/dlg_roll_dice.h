/**
 * @file dlg_roll_dice.h
 * @ingroup GameDialogs
 */
//! \todo Document this file.

#ifndef DLG_ROLL_DICE_H
#define DLG_ROLL_DICE_H

#include <QDialog>
#include <qtmetamacros.h>
#include <sys/types.h>

class QDialogButtonBox;
class QLabel;
class QSpinBox;
class QWidget;

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
