/**
 * @file abstract_card_item.h
 * @ingroup GameGraphicsCards
 * @brief Base class for graphical card items, providing shared rendering, identity, and interaction logic.
 */

#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "../../game/board/abstract_card_state.h"
#include "../card_dimensions.h"
#include "arrow_target.h"
#include "graphics_item_type.h"

#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

class PlayerLogic;

class AbstractCardItem : public ArrowTarget
{
    Q_OBJECT
protected:
    AbstractCardState *cardState;
    int tapAngle;
    QColor bgColor;

private:
    bool isHovered;
    qreal realZValue;
private slots:
    void pixmapUpdated();

signals:
    void hovered(AbstractCardItem *card);
    void showCardInfoPopup(const QPoint &pos, const CardRef &cardRef);
    void deleteCardInfoPopup(QString cardName);
    void sigPixmapUpdated();
    void cardShiftClicked(QString cardName);
    void rightClicked(AbstractCardItem *card, QPoint screenPos);
    void playSelected(AbstractCardItem *card);
    void playSelectedFaceDown(AbstractCardItem *card);
    void hideSelected(AbstractCardItem *card);
    void selectionChanged(AbstractCardItem *card, bool selected);

public:
    enum
    {
        Type = typeCard
    };
    int type() const override
    {
        return Type;
    }
    explicit AbstractCardItem(AbstractCardState *_state, QGraphicsItem *parent = nullptr);
    ~AbstractCardItem() override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QSizeF getTranslatedSize(QPainter *painter) const;
    void paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    qreal getRealZValue() const
    {
        return realZValue;
    }
    void setRealZValue(qreal _zValue);
    void setHovered(bool _hovered);
    void onCardInfoRefreshed();
    void onCardColorChanged();
    bool getIsHovered() const
    {
        return isHovered;
    }
    void processHoverEvent();
    void deleteCardInfoPopup()
    {
        // emit deleteCardInfoPopup(cardRef.name);
    }
    AbstractCardState *getState() const
    {
        return cardState;
    }

protected:
    void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    void cacheBgColor();
};

#endif
