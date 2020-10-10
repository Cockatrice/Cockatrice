#ifndef CARDINFOTEXT_H
#define CARDINFOTEXT_H

#include "carddatabase.h"

#include <QFrame>
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
    explicit CardInfoText(QWidget *parent = nullptr);
    void retranslateUi();
    void setInvalidCardName(const QString &cardName);

signals:
    void linkActivated(const QString &link);
public slots:
    void setCard(CardInfoPtr card);
};

#endif
