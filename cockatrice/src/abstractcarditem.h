#ifndef ABSTRACTCARDITEM_H
#define ABSTRACTCARDITEM_H

#include "arrowtarget.h"

class CardInfo;
class CardInfoWidget;
class Player;
class QTimer;

const int CARD_WIDTH = 72;
const int CARD_HEIGHT = 102;

class AbstractCardItem : public ArrowTarget {
	Q_OBJECT
protected:
	CardInfo *info;
	CardInfoWidget *infoWidget;
	QString name;
	bool tapped;
	int tapAngle;
	QString color;
private:
	QTimer *animationTimer;
	bool isHovered;
	qreal realZValue;
private slots:
	void animationEvent();
	void pixmapUpdated();
signals:
	void hovered(AbstractCardItem *card);
	void showCardInfoPopup(QPoint pos, QString cardName);
	void deleteCardInfoPopup();
public:
	enum { Type = typeCard };
	int type() const { return Type; }
	AbstractCardItem(const QString &_name = QString(), Player *_owner = 0, QGraphicsItem *parent = 0);
	~AbstractCardItem();
	QRectF boundingRect() const;
	void paintPicture(QPainter *painter, int angle);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	CardInfo *getInfo() const { return info; }
	QString getName() const { return name; }
	void setName(const QString &_name = QString());
	qreal getRealZValue() const { return realZValue; }
	void setRealZValue(qreal _zValue);
	void setHovered(bool _hovered);
	QString getColor() const { return color; }
	void setColor(const QString &_color);
	bool getTapped() const { return tapped; }
	void setTapped(bool _tapped, bool canAnimate = false);
	void processHoverEvent();
protected:
	QSizeF getTranslatedSize(QPainter *painter) const;
	void transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
};
 
#endif
