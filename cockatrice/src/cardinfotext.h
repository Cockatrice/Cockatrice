#ifndef CARDINFOTEXT_H
#define CARDINFOTEXT_H

#include <QFrame>

#include "carddatabase.h"
class QLabel;
class QTextEdit;

class CardInfoText : public QFrame
{
    Q_OBJECT

private:
    QLabel *nameLabel1, *nameLabel2;
    QLabel *manacostLabel1, *manacostLabel2;
    QLabel *colorLabel1, *colorLabel2;
    QLabel *cardtypeLabel1, *cardtypeLabel2;
    QLabel *powtoughLabel1, *powtoughLabel2;
    QLabel *loyaltyLabel1, *loyaltyLabel2;
    QTextEdit *textLabel;

    CardInfoPtr info;

    void resetLabels();

public:
    CardInfoText(QWidget *parent = 0);
    void retranslateUi();
    void setInvalidCardName(const QString &cardName);

public slots:
    void setCard(CardInfoPtr card);
};

#endif
