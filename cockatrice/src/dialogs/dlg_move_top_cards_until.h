#ifndef DLG_MOVE_TOP_CARDS_UNTIL_H
#define DLG_MOVE_TOP_CARDS_UNTIL_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

class FilterString;

class DlgMoveTopCardsUntil : public QDialog
{
    Q_OBJECT

    QLabel *exprLabel, *numberOfHitsLabel;
    QLineEdit *exprEdit;
    QSpinBox *numberOfHitsEdit;
    QDialogButtonBox *buttonBox;
    QCheckBox *autoPlayCheckBox;

    void validateAndAccept();
    bool validateMatchExists(const FilterString &filterString);

public:
    explicit DlgMoveTopCardsUntil(QWidget *parent = nullptr,
                                  QString expr = QString(),
                                  uint numberOfHits = 1,
                                  bool autoPlay = false);
    QString getExpr() const;
    uint getNumberOfHits() const;
    bool isAutoPlay() const;
};

#endif // DLG_MOVE_TOP_CARDS_UNTIL_H
