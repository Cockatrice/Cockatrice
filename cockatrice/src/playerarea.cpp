#include "playerarea.h"
#include "player.h"
#include "tablezone.h"
#include "handzone.h"
#include "libraryzone.h"
#include "gravezone.h"
#include "rfgzone.h"
#include "sideboardzone.h"
#include <QPainter>

PlayerArea::PlayerArea(Player *_player, QGraphicsItem *parent)
	: QGraphicsItem(parent), player(_player)
{
 	QPointF base = QPointF(20, 50);
	
	LibraryZone *deck = new LibraryZone(_player, this);
	deck->setPos(base);

	qreal h = deck->boundingRect().height() + 20;

	GraveZone *grave = new GraveZone(_player, this);
	grave->setPos(base + QPointF(0, h));

	RfgZone *rfg = new RfgZone(_player, this);
	rfg->setPos(base + QPointF(0, 2 * h));

	SideboardZone *sb = new SideboardZone(_player, this);
	sb->setVisible(false);

	base = QPointF(deck->boundingRect().width() + 40, 0);

	CardZone *hand = new HandZone(_player, this);
	hand->setPos(base);
	base += QPointF(hand->boundingRect().width(), 0);

	CardZone *table = new TableZone(_player, this);
	table->setPos(base);
	
	bRect = QRectF(0, 0, base.x() + table->boundingRect().width(), base.y() + table->boundingRect().height());
}

PlayerArea::~PlayerArea()
{

}

QRectF PlayerArea::boundingRect() const
{
	return bRect;
}

void PlayerArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	painter->fillRect(boundingRect(), QColor(200, 200, 200));
	
	painter->setFont(QFont("Times", 16, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->drawText(QRectF(0, 0, CARD_WIDTH + 40, 40), Qt::AlignCenter, player->getName());

	painter->restore();
}
