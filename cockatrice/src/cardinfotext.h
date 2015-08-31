#ifndef CARDINFOTEXT_H
#define CARDINFOTEXT_H

#include <QFrame>

class QLabel;
class QTextEdit;
class CardInfo;

class CardInfoText : public QFrame {
    Q_OBJECT

private:
    QLabel *nameLabel1, *nameLabel2;
    QLabel *manacostLabel1, *manacostLabel2;
    QLabel *colorLabel1, *colorLabel2;
    QLabel *cardtypeLabel1, *cardtypeLabel2;
    QLabel *powtoughLabel1, *powtoughLabel2;
    QLabel *loyaltyLabel1, *loyaltyLabel2;
    QTextEdit *textLabel;

    CardInfo *info;

public:
    CardInfoText(QWidget *parent = 0);
    void retranslateUi();

public slots:
    void setCard(CardInfo *card);
};

#endif
