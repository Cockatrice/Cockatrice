#include "counter.h"
#include "player.h"
#include "protocol_items.h"
#include <QtGui>

Counter::Counter(Player *_player, int _id, const QString &_name, QColor _color, int _radius, int _value, QGraphicsItem *parent)
	: QGraphicsItem(parent), player(_player), id(_id), name(_name), color(_color), radius(_radius), value(_value), aDec(0), aInc(0)
{
	if (radius > Player::counterAreaWidth / 2)
		radius = Player::counterAreaWidth / 2;
	if (player->getLocal()) {
		menu = new QMenu(name);
		aSet = new QAction(this);
		connect(aSet, SIGNAL(triggered()), this, SLOT(setCounter()));
		menu->addAction(aSet);
		menu->addSeparator();
		for (int i = -10; i <= 10; ++i)
			if (i == 0)
				menu->addSeparator();
			else {
				QAction *aIncrement = new QAction(QString(i < 0 ? "%1" : "+%1").arg(i), this);
				if (i == -1)
					aDec = aIncrement;
				else if (i == 1)
					aInc = aIncrement;
				aIncrement->setData(i);
				connect(aIncrement, SIGNAL(triggered()), this, SLOT(incrementCounter()));
				menu->addAction(aIncrement);
			}
	} else
		menu = 0;
	
	retranslateUi();
}

Counter::~Counter()
{
	delete menu;
}

void Counter::retranslateUi()
{
	if (menu) {
		aSet->setText(tr("&Set counter..."));
		if (name == "life") {
			aSet->setShortcut(tr("Ctrl+L"));
			aDec->setShortcut(tr("F11"));
			aInc->setShortcut(tr("F12"));
		}
	}
}

QRectF Counter::boundingRect() const
{
	return QRectF(0, 0, radius * 2, radius * 2);
}

void Counter::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->setBrush(QBrush(color));
	painter->drawEllipse(boundingRect());
	if (value) {
		QFont f("Serif");
		f.setPixelSize(radius * 0.8);
		f.setWeight(QFont::Bold);
		painter->setFont(f);
		painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(value));
	}
}

void Counter::setValue(int _value)
{
	value = _value;
	update();
}

void Counter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		player->sendGameCommand(new Command_IncCounter(-1, id, 1));
		event->accept();
	} else if (event->button() == Qt::RightButton) {
		player->sendGameCommand(new Command_IncCounter(-1, id, -1));
		event->accept();
	} else if (event->button() == Qt::MidButton) {
		if (menu)
			menu->exec(event->screenPos());
		event->accept();
	} else
		event->ignore();
}

void Counter::incrementCounter()
{
	int delta = static_cast<QAction *>(sender())->data().toInt();
	player->sendGameCommand(new Command_IncCounter(-1, id, delta));
}

void Counter::setCounter()
{
	bool ok;
	int newValue = QInputDialog::getInteger(0, tr("Set counter"), tr("New value for counter '%1':").arg(name), value, 0, 2000000000, 1, &ok);
	if (ok)
		player->sendGameCommand(new Command_SetCounter(-1, id, newValue));
}
