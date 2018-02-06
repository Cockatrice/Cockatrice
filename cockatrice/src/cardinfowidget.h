#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QComboBox>
#include <QFrame>
#include <QStringList>

#include "carddatabase.h"

class CardInfoPicture;
class CardInfoText;
class AbstractCardItem;

class CardInfoWidget : public QFrame
{
    Q_OBJECT

private:
    qreal aspectRatio;
    CardInfoPtr info;
    CardInfoPicture *pic;
    CardInfoText *text;

public:
    CardInfoWidget(const QString &cardName, QWidget *parent = 0, Qt::WindowFlags f = 0);

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
};

#endif
