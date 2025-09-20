#ifndef CARDINFOTEXT_H
#define CARDINFOTEXT_H

#include "../../../card/card_info.h"

#include <QFrame>
class QLabel;
class QTextEdit;

class CardInfoTextWidget : public QFrame
{
    Q_OBJECT

private:
    QLabel *nameLabel;
    QTextEdit *textLabel;
    CardInfoPtr info;

public:
    explicit CardInfoTextWidget(QWidget *parent = nullptr);
    void retranslateUi();
    void setInvalidCardName(const QString &cardName);

signals:
    void linkActivated(const QString &link);
public slots:
    void setCard(CardInfoPtr card);
};

#endif
