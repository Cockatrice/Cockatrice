#include "playerarea.h"
#include "player.h"
#include "tablezone.h"
#include "handzone.h"
#include "pilezone.h"
#include "counter.h"
#include <QPainter>
#include <QSettings>

PlayerArea::PlayerArea(Player *_player, QGraphicsItem *parent)
	: QGraphicsItem(parent), player(_player)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/playerarea").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	setCacheMode(DeviceCoordinateCache);
	
	QPointF base = QPointF(55, 50);

	PileZone *deck = new PileZone(_player, "deck", true, false, this);
	deck->setPos(base);

	qreal h = deck->boundingRect().height() + 20;

	PileZone *grave = new PileZone(_player, "grave", false, true, this);
	grave->setPos(base + QPointF(0, h));

	PileZone *rfg = new PileZone(_player, "rfg", false, true, this);
	rfg->setPos(base + QPointF(0, 2 * h));

	PileZone *sb = new PileZone(_player, "sb", false, true, this);
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
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(200, 200, 200));
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));

	QString nameStr = player->getName();
	QFont font("Times");
	font.setPixelSize(20);
//	font.setWeight(QFont::Bold);
	
	int totalWidth = CARD_WIDTH + 60;
	
	if (player->getActive()) {
		QFontMetrics fm(font);
		double w = fm.width(nameStr) * 1.7;
		double h = fm.height() * 1.7;
		if (w < h)
			w = h;
		
		painter->setPen(Qt::transparent);
		QRadialGradient grad(QPointF(0.5, 0.5), 0.5);
		grad.setCoordinateMode(QGradient::ObjectBoundingMode);
		grad.setColorAt(0, QColor(150, 200, 150, 255));
		grad.setColorAt(0.7, QColor(150, 200, 150, 255));
		grad.setColorAt(1, QColor(150, 150, 150, 0));
		painter->setBrush(QBrush(grad));
		
		painter->drawEllipse(QRectF(((double) (totalWidth - w)) / 2, 0, w, h));
	}
	painter->setFont(font);
	painter->setPen(QPen(Qt::black));
	painter->drawText(QRectF(0, 0, totalWidth, 40), Qt::AlignCenter, nameStr);
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
