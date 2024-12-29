#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "../board/arrow_target.h"
#include "card_database.h"

class Player;

const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;

class AbstractCardItem : public ArrowTarget
{
    Q_OBJECT
protected:
    CardInfoPtr info;
    int id;
    QString name;
    QString providerId;
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
    void cardInfoUpdated();
    void callUpdate()
    {
        update();
    }
signals:
    void hovered(AbstractCardItem *card);
    void showCardInfoPopup(const QPoint &pos, const QString &cardName, const QString &providerId);
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
    AbstractCardItem(QGraphicsItem *parent = nullptr,
                     const QString &_name = QString(),
                     const QString &_providerId = QString(),
                     Player *_owner = nullptr,
                     int _id = -1);
    ~AbstractCardItem();
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QSizeF getTranslatedSize(QPainter *painter) const;
    void paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    CardInfoPtr getInfo() const
    {
        return info;
    }
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
        return name;
    }
    void setName(const QString &_name = QString());
    QString getProviderId() const
    {
        return providerId;
    }
    void setProviderId(const QString &_providerId = QString());
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
        emit deleteCardInfoPopup(name);
    }

protected:
    void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    void cacheBgColor();
};

#endif
