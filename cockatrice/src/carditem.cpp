#include <QApplication>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "carditem.h"
#include "carddragitem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "tablezone.h"
#include "player.h"
#include "arrowitem.h"
#include "main.h"
#include "protocol_datastructures.h"
#include "settingscache.h"
#include "tab_game.h"

CardItem::CardItem(Player *_owner, const QString &_name, int _cardid, bool _revealedCard, QGraphicsItem *parent)
	: AbstractCardItem(_name, _owner, parent), zone(0), id(_cardid), revealedCard(_revealedCard), attacking(false), facedown(false), destroyOnZoneChange(false), doesntUntap(false), dragItem(0), attachedTo(0)
{
	owner->addCard(this);

	aTap = new QAction(this);
	aTap->setData(0);
	connect(aTap, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	aUntap = new QAction(this);
	aUntap->setData(1);
	connect(aUntap, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	aDoesntUntap = new QAction(this);
	aDoesntUntap->setData(2);
	connect(aDoesntUntap, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	aAttach = new QAction(this);
	connect(aAttach, SIGNAL(triggered()), this, SLOT(actAttach()));
	aUnattach = new QAction(this);
	connect(aUnattach, SIGNAL(triggered()), this, SLOT(actUnattach()));
	aSetPT = new QAction(this);
	connect(aSetPT, SIGNAL(triggered()), this, SLOT(actSetPT()));
	aSetAnnotation = new QAction(this);
	connect(aSetAnnotation, SIGNAL(triggered()), this, SLOT(actSetAnnotation()));
	aFlip = new QAction(this);
	aFlip->setData(3);
	connect(aFlip, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	aClone = new QAction(this);
	aClone->setData(4);
	connect(aClone, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	aMoveToTopLibrary = new QAction(this);
	aMoveToTopLibrary->setData(5);
	aMoveToBottomLibrary = new QAction(this);
	aMoveToBottomLibrary->setData(6);
	aMoveToGraveyard = new QAction(this);
	aMoveToGraveyard->setData(7);
	aMoveToExile = new QAction(this);
	aMoveToExile->setData(8);
	connect(aMoveToTopLibrary, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	connect(aMoveToBottomLibrary, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	connect(aMoveToGraveyard, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	connect(aMoveToExile, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
	
	aPlay = new QAction(this);
	connect(aPlay, SIGNAL(triggered()), this, SLOT(actPlay()));
	aHide = new QAction(this);
	connect(aHide, SIGNAL(triggered()), this, SLOT(actHide()));
		
	for (int i = 0; i < 3; ++i) {
		QAction *tempAddCounter = new QAction(this);
		tempAddCounter->setData(9 + i * 1000);
		QAction *tempRemoveCounter = new QAction(this);
		tempRemoveCounter->setData(10 + i * 1000);
		QAction *tempSetCounter = new QAction(this);
		tempSetCounter->setData(11 + i * 1000);
		aAddCounter.append(tempAddCounter);
		aRemoveCounter.append(tempRemoveCounter);
		aSetCounter.append(tempSetCounter);
		connect(tempAddCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
		connect(tempRemoveCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
		connect(tempSetCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
	}
	cardMenu = new QMenu;
	moveMenu = new QMenu;
	
	retranslateUi();
	updateCardMenu();
}

CardItem::~CardItem()
{
	prepareDelete();
	
	delete cardMenu;
	cardMenu = 0;
	delete moveMenu;
	moveMenu = 0;
	
	deleteDragItem();
}

void CardItem::prepareDelete()
{
	if (owner) {
		if (owner->getCardMenu() == cardMenu)
			owner->setCardMenu(0);
		owner = 0;
	}
	
	while (!attachedCards.isEmpty()) {
		attachedCards.first()->setZone(0); // so that it won't try to call reorganizeCards()
		attachedCards.first()->setAttachedTo(0);
	}
	
	if (attachedTo) {
		attachedTo->removeAttachedCard(this);
		attachedTo = 0;
	}
}

void CardItem::deleteLater()
{
	prepareDelete();
	AbstractCardItem::deleteLater();
}

void CardItem::setZone(CardZone *_zone)
{
	zone = _zone;
	updateCardMenu();
}

void CardItem::updateCardMenu()
{
	cardMenu->clear();
	
	if (revealedCard)
		cardMenu->addAction(aHide);
	else if (owner->getLocal()) {
		if (zone) {
			if (zone->getName() == "table") {
				cardMenu->addAction(aTap);
				cardMenu->addAction(aUntap);
				cardMenu->addAction(aDoesntUntap);
				cardMenu->addAction(aFlip);
				cardMenu->addSeparator();
				cardMenu->addAction(aAttach);
				if (attachedTo)
					cardMenu->addAction(aUnattach);
				cardMenu->addSeparator();
				cardMenu->addAction(aSetPT);
				cardMenu->addAction(aSetAnnotation);
				cardMenu->addSeparator();
				cardMenu->addAction(aClone);
				
				for (int i = 0; i < aAddCounter.size(); ++i) {
					cardMenu->addSeparator();
					cardMenu->addAction(aAddCounter[i]);
					cardMenu->addAction(aRemoveCounter[i]);
					cardMenu->addAction(aSetCounter[i]);
				}
				cardMenu->addSeparator();
			} else {
				cardMenu->addAction(aPlay);
			}
		}
		
		moveMenu->clear();
		moveMenu->addAction(aMoveToTopLibrary);
		moveMenu->addAction(aMoveToBottomLibrary);
		moveMenu->addAction(aMoveToGraveyard);
		moveMenu->addAction(aMoveToExile);
		cardMenu->addMenu(moveMenu);
	}
}

void CardItem::retranslateUi()
{
	aPlay->setText(tr("&Play"));
	aHide->setText(tr("&Hide"));
	
	aTap->setText(tr("&Tap"));
	aUntap->setText(tr("&Untap"));
	aDoesntUntap->setText(tr("Toggle &normal untapping"));
	aFlip->setText(tr("&Flip"));
	aClone->setText(tr("&Clone"));
	aAttach->setText(tr("&Attach to card..."));
	aAttach->setShortcut(tr("Ctrl+A"));
	aUnattach->setText(tr("Unattac&h"));
	aSetPT->setText(tr("Set &P/T..."));
	aSetAnnotation->setText(tr("&Set annotation..."));
	QStringList counterColors;
	counterColors.append(tr("red"));
	counterColors.append(tr("yellow"));
	counterColors.append(tr("green"));
	for (int i = 0; i < aAddCounter.size(); ++i)
		aAddCounter[i]->setText(tr("&Add counter (%1)").arg(counterColors[i]));
	for (int i = 0; i < aRemoveCounter.size(); ++i)
		aRemoveCounter[i]->setText(tr("&Remove counter (%1)").arg(counterColors[i]));
	for (int i = 0; i < aSetCounter.size(); ++i)
		aSetCounter[i]->setText(tr("&Set counters (%1)...").arg(counterColors[i]));
	aMoveToTopLibrary->setText(tr("&top of library"));
	aMoveToBottomLibrary->setText(tr("&bottom of library"));
	aMoveToGraveyard->setText(tr("&graveyard"));
	aMoveToGraveyard->setShortcut(tr("Ctrl+Del"));
	aMoveToExile->setText(tr("&exile"));
	
	moveMenu->setTitle(tr("&Move to"));
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();
	AbstractCardItem::paint(painter, option, widget);
	
	int i = 0;
	QMapIterator<int, int> counterIterator(counters);
	while (counterIterator.hasNext()) {
		counterIterator.next();
		QColor color;
		color.setHsv(counterIterator.key() * 60, 150, 255);
		
		paintNumberEllipse(counterIterator.value(), 14, color, i, counters.size(), painter);
		++i;
	}
	if (!pt.isEmpty()) {
		painter->save();
		QSizeF translatedSize = getTranslatedSize(painter);

		qreal scaleFactor = translatedSize.width() / boundingRect().width();
		transformPainter(painter, translatedSize, tapAngle);
		painter->setBackground(Qt::black);
		painter->setBackgroundMode(Qt::OpaqueMode);
		painter->setPen(Qt::white);
		
		painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 8 * scaleFactor, translatedSize.height() - 8 * scaleFactor), Qt::AlignRight | Qt::AlignBottom, pt);
		painter->restore();
	}
	if (getBeingPointedAt())
		painter->fillRect(boundingRect(), QBrush(QColor(255, 0, 0, 100)));
	painter->restore();
}

void CardItem::setAttacking(bool _attacking)
{
	attacking = _attacking;
	update();
}

void CardItem::setFaceDown(bool _facedown)
{
	facedown = _facedown;
	if (facedown)
		setName(QString());
	update();
}

void CardItem::setCounter(int _id, int _value)
{
	if (_value)
		counters.insert(_id, _value);
	else
		counters.remove(_id);
	update();
}

void CardItem::setAnnotation(const QString &_annotation)
{
	annotation = _annotation;
	setToolTip(annotation);
	update();
}

void CardItem::setDoesntUntap(bool _doesntUntap)
{
	doesntUntap = _doesntUntap;
}

void CardItem::setPT(const QString &_pt)
{
	pt = _pt;
	update();
}

void CardItem::setAttachedTo(CardItem *_attachedTo)
{
	if (attachedTo)
		attachedTo->removeAttachedCard(this);
	
	gridPoint.setX(-1);
	attachedTo = _attachedTo;
	if (attachedTo) {
		setParentItem(attachedTo->getZone());
		attachedTo->addAttachedCard(this);
		if (zone != attachedTo->getZone())
			attachedTo->getZone()->reorganizeCards();
	} else
		setParentItem(zone);

	if (zone)
		zone->reorganizeCards();
	
	updateCardMenu();
}

void CardItem::resetState()
{
	attacking = false;
	facedown = false;
	counters.clear();
	pt.clear();
	annotation.clear();
	attachedTo = 0;
	attachedCards.clear();
	setTapped(false);
	setDoesntUntap(false);
	update();
}

void CardItem::processCardInfo(ServerInfo_Card *info)
{
	counters.clear();
	const QList<ServerInfo_CardCounter *> &_counterList = info->getCounters();
	for (int i = 0; i < _counterList.size(); ++i)
		counters.insert(_counterList[i]->getId(), _counterList[i]->getValue());
	
	setId(info->getId());
	setName(info->getName());
	setAttacking(info->getAttacking());
	setPT(info->getPT());
	setAnnotation(info->getAnnotation());
	setColor(info->getColor());
	setTapped(info->getTapped());
	setDestroyOnZoneChange(info->getDestroyOnZoneChange());
}

CardDragItem *CardItem::createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown)
{
	deleteDragItem();
	dragItem = new CardDragItem(this, _id, _pos, faceDown);
	scene()->addItem(dragItem);
	dragItem->updatePosition(_scenePos);

	return dragItem;
}

void CardItem::deleteDragItem()
{
	dragItem->deleteLater();
	dragItem = NULL;
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::RightButton)) {
		if ((event->screenPos() - event->buttonDownScreenPos(Qt::RightButton)).manhattanLength() < 2 * QApplication::startDragDistance())
			return;
		if (static_cast<TabGame *>(owner->parent())->getSpectator())
			return;
		
		QColor arrowColor = Qt::red;
		if (event->modifiers().testFlag(Qt::ControlModifier))
			arrowColor = Qt::yellow;
		else if (event->modifiers().testFlag(Qt::AltModifier))
			arrowColor = Qt::blue;
		else if (event->modifiers().testFlag(Qt::ShiftModifier))
			arrowColor = Qt::green;
		
		Player *arrowOwner = static_cast<TabGame *>(owner->parent())->getActiveLocalPlayer();
		ArrowDragItem *arrow = new ArrowDragItem(arrowOwner, this, arrowColor);
		scene()->addItem(arrow);
		arrow->grabMouse();
		
		QListIterator<QGraphicsItem *> itemIterator(scene()->selectedItems());
		while (itemIterator.hasNext()) {
			CardItem *c = qgraphicsitem_cast<CardItem *>(itemIterator.next());
			if (!c || (c == this))
				continue;
			if (c->getZone() != zone)
				continue;
			
			ArrowDragItem *childArrow = new ArrowDragItem(arrowOwner, c, arrowColor);
			scene()->addItem(childArrow);
			arrow->addChildArrow(childArrow);
		}
	} else if (event->buttons().testFlag(Qt::LeftButton)) {
		if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < 2 * QApplication::startDragDistance())
			return;
		if (!owner->getLocal())
			return;
		
		bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier) || facedown;
	
		createDragItem(id, event->pos(), event->scenePos(), faceDown);
		dragItem->grabMouse();
		
		QList<QGraphicsItem *> sel = scene()->selectedItems();
		int j = 0;
		for (int i = 0; i < sel.size(); i++) {
			CardItem *c = (CardItem *) sel.at(i);
			if (c == this)
				continue;
			++j;
			QPointF childPos;
			if (zone->getHasCardAttr())
				childPos = c->pos() - pos();
			else
				childPos = QPointF(j * CARD_WIDTH / 2, 0);
			CardDragItem *drag = new CardDragItem(c, c->getId(), childPos, false, dragItem);
			drag->setPos(dragItem->pos() + childPos);
			scene()->addItem(drag);
		}
	}
	setCursor(Qt::OpenHandCursor);
}

void CardItem::playCard(bool faceDown)
{
	// Do nothing if the card belongs to another player
	if (!owner->getLocal())
		return;

	TableZone *tz = qobject_cast<TableZone *>(zone);
	if (tz)
		tz->toggleTapped();
	else
		zone->getPlayer()->playCard(this, faceDown, info->getCipt());
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		if (cardMenu)
			if (!cardMenu->isEmpty())
				cardMenu->exec(event->screenPos());
	} else if ((event->button() == Qt::LeftButton) && !settingsCache->getDoubleClickToPlay()) {
		setCursor(Qt::OpenHandCursor);
		if (revealedCard)
			actHide();
		else
			playCard(event->modifiers().testFlag(Qt::ShiftModifier));
	}

	AbstractCardItem::mouseReleaseEvent(event);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	if (settingsCache->getDoubleClickToPlay()) {
		if (revealedCard)
			actHide();
		else
			playCard(event->modifiers().testFlag(Qt::ShiftModifier));
	}
	event->accept();
}

QVariant CardItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if ((change == ItemSelectedHasChanged) && owner) {
		if (value == true)
			owner->setCardMenu(cardMenu);
		else if (owner->getCardMenu() == cardMenu)
			owner->setCardMenu(0);
	}
	return QGraphicsItem::itemChange(change, value);
}

void CardItem::cardMenuAction()
{
	owner->cardMenuAction(static_cast<QAction *>(sender()));
}

void CardItem::actAttach()
{
	owner->actAttach(static_cast<QAction *>(sender()));
}

void CardItem::actUnattach()
{
	owner->actUnattach(static_cast<QAction *>(sender()));
}

void CardItem::actSetPT()
{
	owner->actSetPT(static_cast<QAction *>(sender()));
}

void CardItem::actSetAnnotation()
{
	owner->actSetAnnotation(static_cast<QAction *>(sender()));
}

void CardItem::actCardCounterTrigger()
{
	owner->actCardCounterTrigger(static_cast<QAction *>(sender()));
}

void CardItem::actPlay()
{
	playCard(false);
}

void CardItem::actHide()
{
	zone->removeCard(this);
}