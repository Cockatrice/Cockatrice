#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "arrowtarget.h"

class CardInfo;
class Player;

const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;

class AbstractCardItem : public ArrowTarget {
    Q_OBJECT
protected:
    CardInfo *info;
    int id;
    QString name;
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
    void callUpdate() { update(); }
signals:
    void hovered(AbstractCardItem *card);
    void showCardInfoPopup(QPoint pos, QString cardName);
    void deleteCardInfoPopup(QString cardName);
    void updateCardMenu(AbstractCardItem *card);
    void sigPixmapUpdated();
public:
    enum { Type = typeCard };
    int type() const { return Type; }
    AbstractCardItem(const QString &_name = QString(), Player *_owner = 0, int _id = -1, QGraphicsItem *parent = 0);
    ~AbstractCardItem();
    QRectF boundingRect() const;
    QSizeF getTranslatedSize(QPainter *painter) const;
    void paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    CardInfo *getInfo() const { return info; }
    int getId() const { return id; }
    void setId(int _id) { id = _id; }
    QString getName() const { return name; }
    void setName(const QString &_name = QString());
    qreal getRealZValue() const { return realZValue; }
    void setRealZValue(qreal _zValue);
    void setHovered(bool _hovered);
    QString getColor() const { return color; }
    void setColor(const QString &_color);
    bool getTapped() const { return tapped; }
    void setTapped(bool _tapped, bool canAnimate = false);
    bool getFaceDown() const { return facedown; }
    void setFaceDown(bool _facedown);
    void processHoverEvent();
    void deleteCardInfoPopup() { emit deleteCardInfoPopup(name); }
protected:
    void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    void cacheBgColor();
};
 
#endif
