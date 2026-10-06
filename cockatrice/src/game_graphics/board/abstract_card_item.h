/**
 * @file abstract_card_item.h
 * @ingroup GameGraphicsCards
 * @brief Base class for graphical card items, providing shared rendering, identity, and interaction logic.
 */

#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "../../game/board/card_state.h"
#include "../animated_item.h"
#include "../card_dimensions.h"
#include "arrow_target.h"
#include "graphics_item_type.h"

class PlayerLogic;

class AbstractCardItem : public ArrowTarget, public IAnimatedItem
{
    Q_OBJECT
protected:
    CardState *state;
    int tapAngle;
    QString color;
    QColor bgColor;

private:
    bool isHovered;
    qreal realZValue;
private slots:
    void pixmapUpdated();
    void onCardInfoChanged();
    void onCardRefChanged(const CardRef &oldCardRef, const CardRef &newCardRef);
    void onTappedChanged(bool newTapped, bool canAnimate);

public slots:
    void refreshCardInfo();

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
    explicit AbstractCardItem(QGraphicsItem *parent = nullptr,
                              const CardRef &cardRef = {},
                              PlayerLogic *_owner = nullptr,
                              int _id = -1);
    ~AbstractCardItem() override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QSizeF getTranslatedSize(QPainter *painter) const;
    void paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] CardState *getState() const
    {
        return state;
    }
    ExactCard getCard() const
    {
        return state->getCard();
    }
    const CardInfo &getCardInfo() const;
    int getId() const
    {
        return state->getId();
    }
    void setId(int _id)
    {
        state->setId(_id);
    }
    QString getName() const
    {
        return state->getCardRef().name;
    }
    QString getProviderId() const
    {
        return state->getCardRef().providerId;
    }
    void setCardRef(const CardRef &_cardRef)
    {
        state->setCardRef(_cardRef);
    }
    CardRef getCardRef() const
    {
        return state->getCardRef();
    }
    qreal getRealZValue() const
    {
        return realZValue;
    }
    void setRealZValue(qreal _zValue);
    void setHovered(bool _hovered);
    bool getIsHovered() const
    {
        return isHovered;
    }
    QString getColor() const
    {
        return color;
    }
    void setColor(const QString &_color);
    bool getTapped() const
    {
        return state->getTapped();
    }
    void setTapped(bool _tapped, bool canAnimate = false)
    {
        state->setTapped(_tapped, canAnimate);
    }
    bool getFaceDown() const
    {
        return state->getFaceDown();
    }
    void setFaceDown(bool _facedown)
    {
        state->setFaceDown(_facedown);
    }
    void processHoverEvent();
    void deleteCardInfoPopup()
    {
        emit deleteCardInfoPopup(state->getCardRef().name);
    }

    /** @brief Default: no per-tick animation. Subclasses override to animate. */
    bool animationEvent() override;

protected:
    void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    void cacheBgColor();
};

#endif
