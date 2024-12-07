#ifndef DLG_MOVE_TOP_CARDS_UNTIL_H
#define DLG_MOVE_TOP_CARDS_UNTIL_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>

class DlgMoveTopCardsUntil : public QDialog
{
    Q_OBJECT

    QLabel *exprLabel, *numberOfHitsLabel;
    QLineEdit *exprEdit;
    QSpinBox *numberOfHitsEdit;
    QDialogButtonBox *buttonBox;

public:
    explicit DlgMoveTopCardsUntil(QWidget *parent = nullptr, QString expr = QString(), uint numberOfHits = 1);
    [[nodiscard]] QString getExpr() const;
    [[nodiscard]] uint getNumberOfHits() const;
};

#endif // DLG_MOVE_TOP_CARDS_UNTIL_H
