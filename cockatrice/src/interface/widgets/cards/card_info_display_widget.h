/**
 * @file card_info_display_widget.h
 * @ingroup CardWidgets
 * @brief TODO: Document this.
 */

#ifndef CARDINFOWIDGET_H
#define CARDINFOWIDGET_H

#include <QComboBox>
#include <QFrame>
#include <QStringList>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

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
