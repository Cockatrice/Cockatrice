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
    QLabel *nameLabel;
    QTextEdit *textLabel;
    CardInfoPtr info;

public:
    CardInfoText(QWidget *parent = 0);
    void retranslateUi();
    void setInvalidCardName(const QString &cardName);

signals:
    void linkActivated(const QString &link);
public slots:
    void setCard(CardInfoPtr card);
};

#endif
