#include "playerarea.h"
#include "player.h"
#include "tablezone.h"
#include "handzone.h"
#include "libraryzone.h"
#include "pilezone.h"
#include "counter.h"
#include <QPainter>

PlayerArea::PlayerArea(Player *_player, QGraphicsItem *parent)
	: QGraphicsItem(parent), player(_player)
{
 	QPointF base = QPointF(55, 50);

	LibraryZone *deck = new LibraryZone(_player, this);
	deck->setPos(base);

	qreal h = deck->boundingRect().height() + 20;

	PileZone *grave = new PileZone(_player, "grave", this);
	grave->setPos(base + QPointF(0, h));

	PileZone *rfg = new PileZone(_player, "rfg", this);
	rfg->setPos(base + QPointF(0, 2 * h));

	PileZone *sb = new PileZone(_player, "sb", this);
	sb->setVisible(false);

	base = QPointF(deck->boundingRect().width() + 60, 0);

	CardZone *hand = new HandZone(_player, this);
	hand->setPos(base);
	base += QPointF(hand->boundingRect().width(), 0);

	CardZone *table = new TableZone(_player, this);
	table->setPos(base);

	bRect = QRectF(0, 0, base.x() + table->boundingRect().width(), base.y() + table->boundingRect().height());
}

PlayerArea::~PlayerArea()
{
	clearCounters();
}

QRectF PlayerArea::boundingRect() const
{
	return bRect;
}

void PlayerArea::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->fillRect(boundingRect(), QColor(200, 200, 200));

	painter->setFont(QFont("Times", 16, QFont::Bold));
	painter->setPen(QPen(Qt::black));
	painter->drawText(QRectF(0, 0, CARD_WIDTH + 60, 40), Qt::AlignCenter, player->getName());
}

Counter *PlayerArea::getCounter(const QString &name, bool remove)
{
	for (int i = 0; i < counterList.size(); i++) {
		Counter *temp = counterList.at(i);
		if (temp->getName() == name) {
			if (remove)
				counterList.removeAt(i);
			return temp;
		}
	}
	return 0;
}

void PlayerArea::addCounter(const QString &name, QColor color, int value)
{
	counterList.append(new Counter(player, name, color, value, this));
	rearrangeCounters();
}

void PlayerArea::delCounter(const QString &name)
{
	delete getCounter(name, true);
	rearrangeCounters();
}

void PlayerArea::clearCounters()
{
	for (int i = 0; i < counterList.size(); i++)
		delete counterList.at(i);
	counterList.clear();
}

void PlayerArea::rearrangeCounters()
{
	const int counterAreaWidth = 55;
	qreal y = 50;
	for (int i = 0; i < counterList.size(); i++) {
		Counter *temp = counterList.at(i);
		QRectF br = temp->boundingRect();
		temp->setPos((counterAreaWidth - br.width()) / 2, y);
		y += br.height() + 10;
	}
}
