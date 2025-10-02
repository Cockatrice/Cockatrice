/**
 * @file card_info_display_widget.h
 * @ingroup CardWidgets
 * @brief TODO: Document this.
 */

#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include "card/card_printing/exact_card.h"
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
    ExactCard exactCard;
    CardInfoPictureWidget *pic;
    CardInfoTextWidget *text;

public:
    explicit CardInfoDisplayWidget(const CardRef &cardRef, QWidget *parent = nullptr, Qt::WindowFlags f = {});

public slots:
    void setCard(const ExactCard &card);
    void setCard(const CardRef &cardRef);
    void setCard(AbstractCardItem *card);

private slots:
    void clear();
};

#endif
