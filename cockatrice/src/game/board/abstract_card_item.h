/**
 * @file abstract_card_item.h
 * @ingroup GameGraphicsCards
 * @brief TODO: Document this.
 */

#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "arrow_target.h"
#include "graphics_item_type.h"

#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

class Player;

const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;

class AbstractCardItem : public ArrowTarget
{
    Q_OBJECT
protected:
    ExactCard exactCard;
    int id;
    CardRef cardRef;
    bool tapped;
    bool facedown;
    int tapAngle;
    QString color;
    QColor bgColor;

private:
    bool isHovered;
    qreal realZValue;
private slots:
    void pixmapUpdated();

public slots:
    void refreshCardInfo();

signals:
    void hovered(AbstractCardItem *card);
    void showCardInfoPopup(const QPoint &pos, const CardRef &cardRef);
    void deleteCardInfoPopup(QString cardName);
    void sigPixmapUpdated();
    void cardShiftClicked(QString cardName);

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
                              Player *_owner = nullptr,
                              int _id = -1);
    ~AbstractCardItem() override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QSizeF getTranslatedSize(QPainter *painter) const;
    void paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    ExactCard getCard() const
    {
        return exactCard;
    }
    const CardInfo &getCardInfo() const;
    int getId() const
    {
        return id;
    }
    void setId(int _id)
    {
        id = _id;
    }
    QString getName() const
    {
        return cardRef.name;
    }
    QString getProviderId() const
    {
        return cardRef.providerId;
    }
    void setCardRef(const CardRef &_cardRef);
    CardRef getCardRef() const
    {
        return cardRef;
    }
    qreal getRealZValue() const
    {
        return realZValue;
    }
    void setRealZValue(qreal _zValue);
    void setHovered(bool _hovered);
    QString getColor() const
    {
        return color;
    }
    void setColor(const QString &_color);
    bool getTapped() const
    {
        return tapped;
    }
    void setTapped(bool _tapped, bool canAnimate = false);
    bool getFaceDown() const
    {
        return facedown;
    }
    void setFaceDown(bool _facedown);
    void processHoverEvent();
    void deleteCardInfoPopup()
    {
        emit deleteCardInfoPopup(cardRef.name);
    }

protected:
    void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    void cacheBgColor();
};

#endif
