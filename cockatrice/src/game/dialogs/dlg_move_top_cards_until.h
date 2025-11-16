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

public:
    explicit DlgMoveTopCardsUntil(QWidget *parent = nullptr,
                                  QStringList exprs = QStringList(),
                                  uint numberOfHits = 1,
                                  bool autoPlay = false);
    [[nodiscard]] QString getExpr() const;
    [[nodiscard]] QStringList getExprs() const;
    [[nodiscard]] uint getNumberOfHits() const;
    [[nodiscard]] bool isAutoPlay() const;
};

#endif // DLG_MOVE_TOP_CARDS_UNTIL_H
