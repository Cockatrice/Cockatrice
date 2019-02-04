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
    explicit CardInfoWidget(const QString &cardName, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
};

#endif
