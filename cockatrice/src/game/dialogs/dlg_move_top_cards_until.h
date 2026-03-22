/**
 * @file dlg_move_top_cards_until.h
 * @ingroup GameDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_MOVE_TOP_CARDS_UNTIL_H
#define DLG_MOVE_TOP_CARDS_UNTIL_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>

class FilterString;

struct MoveTopCardsUntilOptions
{
    QStringList exprs = {};
    int numberOfHits = 1;
    bool autoPlay = false;
};

class DlgMoveTopCardsUntil : public QDialog
{
    Q_OBJECT

    QLabel *exprLabel, *numberOfHitsLabel;
    QComboBox *exprComboBox;
    QSpinBox *numberOfHitsEdit;
    QDialogButtonBox *buttonBox;
    QCheckBox *autoPlayCheckBox;

    void validateAndAccept();
    bool validateMatchExists(const FilterString &filterString);

    [[nodiscard]] QStringList getExprs() const;

public:
    explicit DlgMoveTopCardsUntil(QWidget *parent = nullptr, const MoveTopCardsUntilOptions &options = {});
    [[nodiscard]] QString getExpr() const;
    [[nodiscard]] MoveTopCardsUntilOptions getOptions() const;
};

#endif // DLG_MOVE_TOP_CARDS_UNTIL_H
