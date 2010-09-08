#ifndef COUNTER_H
#define COUNTER_H

#include <QGraphicsItem>

class Player;
class QMenu;
class QAction;

class Counter : public QObject, public QGraphicsItem {
	Q_OBJECT
private:
	Player *player;
	int id;
	QString name;
	QColor color;
	int radius;
	int value;
	
	QAction *aSet, *aDec, *aInc;
	QMenu *menu;
	bool dialogSemaphore, deleteAfterDialog;
private slots:
	void incrementCounter();
	void setCounter();
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	Counter(Player *_player, int _id, const QString &_name, QColor _color, int _radius, int _value, QGraphicsItem *parent = 0);
	~Counter();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	QMenu *getMenu() const { return menu; }
	void retranslateUi();
	
	int getId() const { return id; }
	QString getName() const { return name; }
	int getValue() const { return value; }
	void setValue(int _value);
	void delCounter();
	
	void setShortcutsActive();
	void setShortcutsInactive();
};

#endif
