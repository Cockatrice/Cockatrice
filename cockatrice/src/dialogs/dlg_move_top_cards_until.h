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
    QString getExpr() const;
    QStringList getExprs() const;
    uint getNumberOfHits() const;
    bool isAutoPlay() const;
};

#endif // DLG_MOVE_TOP_CARDS_UNTIL_H
