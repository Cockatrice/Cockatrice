#include "abstractcounter.h"
#include "player.h"
#include "protocol_items.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

AbstractCounter::AbstractCounter(Player *_player, int _id, const QString &_name, bool _shownInCounterArea, int _value, QGraphicsItem *parent)
	: QGraphicsItem(parent), player(_player), id(_id), name(_name), value(_value), hovered(false), aDec(0), aInc(0), dialogSemaphore(false), deleteAfterDialog(false), shownInCounterArea(_shownInCounterArea)
{
	setAcceptsHoverEvents(true);
	
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

AbstractCounter::~AbstractCounter()
{
	delete menu;
}

void AbstractCounter::delCounter()
{
	if (dialogSemaphore)
		deleteAfterDialog = true;
	else
		deleteLater();
}

void AbstractCounter::retranslateUi()
{
	if (menu) {
		aSet->setText(tr("&Set counter..."));
	}
}

void AbstractCounter::setShortcutsActive()
{
	if (name == "life") {
		aSet->setShortcut(tr("Ctrl+L"));
		aDec->setShortcut(tr("F11"));
		aInc->setShortcut(tr("F12"));
	}
}

void AbstractCounter::setShortcutsInactive()
{
	if (name == "life") {
		aSet->setShortcut(QKeySequence());
		aDec->setShortcut(QKeySequence());
		aInc->setShortcut(QKeySequence());
	}
}

void AbstractCounter::setValue(int _value)
{
	value = _value;
	update();
}

void AbstractCounter::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void AbstractCounter::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	hovered = true;
	update();
}

void AbstractCounter::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	hovered = false;
	update();
}

void AbstractCounter::incrementCounter()
{
	int delta = static_cast<QAction *>(sender())->data().toInt();
	player->sendGameCommand(new Command_IncCounter(-1, id, delta));
}

void AbstractCounter::setCounter()
{
	bool ok;
	dialogSemaphore = true;
	int newValue = QInputDialog::getInteger(0, tr("Set counter"), tr("New value for counter '%1':").arg(name), value, 0, 2000000000, 1, &ok);
	if (deleteAfterDialog) {
		deleteLater();
		return;
	}
	dialogSemaphore = false;
	if (ok)
		player->sendGameCommand(new Command_SetCounter(-1, id, newValue));
}
