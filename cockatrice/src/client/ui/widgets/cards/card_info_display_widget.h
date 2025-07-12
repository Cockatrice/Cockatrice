#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include "../../../../game/cards/card_info.h"
#include "card_ref.h"

#include <QComboBox>
#include <QFrame>
#include <QStringList>

class CardInfoPictureWidget;
class CardInfoTextWidget;
class AbstractCardItem;

class CardInfoDisplayWidget : public QFrame
{
    Q_OBJECT

private:
    qreal aspectRatio;
    CardInfoPtr info;
    CardInfoPictureWidget *pic;
    CardInfoTextWidget *text;

public:
    explicit CardInfoDisplayWidget(const CardRef &cardRef, QWidget *parent = nullptr, Qt::WindowFlags f = {});

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const CardRef &cardRef);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
};

#endif
