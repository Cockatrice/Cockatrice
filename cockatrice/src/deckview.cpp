#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include "deckview.h"
#include "decklist.h"
#include "carddatabase.h"
#include "settingscache.h"
#include "main.h"

DeckViewCardDragItem::DeckViewCardDragItem(DeckViewCard *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag)
	: AbstractCardDragItem(_item, _hotSpot, parentDrag), currentZone(0)
{
}

void DeckViewCardDragItem::updatePosition(const QPointF &cursorScenePos)
{
	QList<QGraphicsItem *> colliding = scene()->items(cursorScenePos);

	DeckViewCardContainer *cursorZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--)
		if ((cursorZone = qgraphicsitem_cast<DeckViewCardContainer *>(colliding.at(i))))
			break;
	if (!cursorZone)
		return;
	currentZone = cursorZone;
	
	QPointF newPos = cursorScenePos;
	if (newPos != pos()) {
		for (int i = 0; i < childDrags.size(); i++)
			childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
		setPos(newPos);
	}
}

void DeckViewCardDragItem::handleDrop(DeckViewCardContainer *target)
{
	DeckViewCard *card = static_cast<DeckViewCard *>(item);
	DeckViewCardContainer *start = static_cast<DeckViewCardContainer *>(item->parentItem());
	start->removeCard(card);
	target->addCard(card);
	card->setParentItem(target);
}

void DeckViewCardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	DeckViewScene *sc = static_cast<DeckViewScene *>(scene());
	QPointF sp = pos();
	sc->removeItem(this);

	if (currentZone) {
		handleDrop(currentZone);
		for (int i = 0; i < childDrags.size(); i++) {
			DeckViewCardDragItem *c = static_cast<DeckViewCardDragItem *>(childDrags[i]);
			c->handleDrop(currentZone);
			sc->removeItem(c);
		}
		
		sc->updateContents();
	}
	
	event->accept();
}

DeckViewCard::DeckViewCard(const QString &_name, const QString &_originZone, QGraphicsItem *parent)
	: AbstractCardItem(_name, 0, parent), originZone(_originZone), dragItem(0)
{
}

DeckViewCard::~DeckViewCard()
{
	delete dragItem;
}

void DeckViewCard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	AbstractCardItem::paint(painter, option, widget);
	
	painter->save();
	QPen pen;//(Qt::DotLine);
	pen.setWidth(3);
	if (originZone == "main")
		pen.setColor(QColor(0, 255, 0));
	else
		pen.setColor(QColor(255, 0, 0));
	painter->setPen(pen);
	painter->drawRect(QRectF(1.5, 1.5, CARD_WIDTH - 3, CARD_HEIGHT - 3));
	painter->restore();
}

void DeckViewCard::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < 2 * QApplication::startDragDistance())
		return;
	
	if (static_cast<DeckViewScene *>(scene())->getLocked())
		return;
	
	delete dragItem;
	dragItem = new DeckViewCardDragItem(this, event->pos());
	scene()->addItem(dragItem);
	dragItem->updatePosition(event->scenePos());
	dragItem->grabMouse();
	
	QList<QGraphicsItem *> sel = scene()->selectedItems();
	int j = 0;
	for (int i = 0; i < sel.size(); i++) {
		DeckViewCard *c = static_cast<DeckViewCard *>(sel.at(i));
		if (c == this)
			continue;
		++j;
		QPointF childPos = QPointF(j * CARD_WIDTH / 2, 0);
		DeckViewCardDragItem *drag = new DeckViewCardDragItem(c, childPos, dragItem);
		drag->setPos(dragItem->pos() + childPos);
		scene()->addItem(drag);
	}
	setCursor(Qt::OpenHandCursor);
}

DeckViewCardContainer::DeckViewCardContainer(const QString &_name)
	: QGraphicsItem(), name(_name), width(0), height(0), maxWidth(0)
{
	QString bgPath = settingsCache->getTableBgPath();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);

	setCacheMode(DeviceCoordinateCache);
}

QRectF DeckViewCardContainer::boundingRect() const
{
	return QRectF(0, 0, width, height);
}

void DeckViewCardContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(0, 0, 100));
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
	painter->setPen(QColor(255, 255, 255, 100));
	painter->drawLine(QPointF(0, separatorY), QPointF(width, separatorY));
	
	painter->setPen(QColor(Qt::white));
	QFont f("Serif");
	f.setStyleHint(QFont::Serif);
	f.setPixelSize(24);
	f.setWeight(QFont::Bold);
	painter->setFont(f);
	painter->drawText(10, 0, width - 20, separatorY, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, InnerDecklistNode::visibleNameFromName(name) + QString(": %1").arg(cards.size()));
}

void DeckViewCardContainer::addCard(DeckViewCard *card)
{
	cards.append(card);
}

void DeckViewCardContainer::removeCard(DeckViewCard *card)
{
	cards.removeAt(cards.indexOf(card));
}

void DeckViewCardContainer::rearrangeItems()
{
	separatorY = 30;
	
	QMap<QString, DeckViewCard *> cardsByType;
	for (int i = 0; i < cards.size(); ++i)
		cardsByType.insertMulti(cards[i]->getInfo()->getMainCardType(), cards[i]);
	
	QList<QString> cardTypeList = cardsByType.uniqueKeys();
	int rows = cardTypeList.size();
	int cols = 0;
	for (int i = 0; i < rows; ++i) {
		QList<DeckViewCard *> row = cardsByType.values(cardTypeList[i]);
		if (row.size() > cols)
			cols = row.size();
		for (int j = 0; j < row.size(); ++j) {
			DeckViewCard *card = row[j];
			card->setPos(j * CARD_WIDTH, separatorY + 10 + i * (CARD_HEIGHT + rowSpacing));
		}
	}
	
	prepareGeometryChange();
	if (cols * CARD_WIDTH > maxWidth)
		width = maxWidth = cols * CARD_WIDTH;
	height = separatorY + 10 + rows * CARD_HEIGHT + rowSpacing * (rows - 1);
}

void DeckViewCardContainer::setWidth(qreal _width)
{
	prepareGeometryChange();
	width = _width;
	update();
}

DeckViewScene::DeckViewScene(QObject *parent)
	: QGraphicsScene(parent), locked(false), deck(0)
{
}

DeckViewScene::~DeckViewScene()
{
}

void DeckViewScene::setDeck(DeckList *_deck)
{
	if (deck)
		delete deck;
	QMapIterator<QString, DeckViewCardContainer *> i(cardContainers);
	while (i.hasNext())
		delete i.next().value();
	cardContainers.clear();
	
	deck = _deck;
	rebuildTree();
	applySideboardPlan(deck->getCurrentSideboardPlan());
	rearrangeItems();
}

void DeckViewScene::rebuildTree()
{
	InnerDecklistNode *listRoot = deck->getRoot();
	for (int i = 0; i < listRoot->size(); i++) {
		InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
		
		DeckViewCardContainer *container = cardContainers.value(currentZone->getName(), 0);
		if (!container) {
			container = new DeckViewCardContainer(currentZone->getName());
			cardContainers.insert(currentZone->getName(), container);
			addItem(container);
		}
		
		for (int j = 0; j < currentZone->size(); j++) {
			DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
			if (!currentCard)
				continue;

			for (int k = 0; k < currentCard->getNumber(); ++k) {
				DeckViewCard *newCard = new DeckViewCard(currentCard->getName(), currentZone->getName(), container);
				container->addCard(newCard);
				emit newCardAdded(newCard);
			}
		}
	}
}

void DeckViewScene::applySideboardPlan(const QList<MoveCardToZone *> &plan)
{
	for (int i = 0; i < plan.size(); ++i) {
		MoveCardToZone *m = plan[i];
		
		DeckViewCardContainer *start = cardContainers.value(m->getStartZone());
		DeckViewCardContainer *target = cardContainers.value(m->getTargetZone());
		if (!start || !target)
			continue;
		
		DeckViewCard *card = 0;
		const QList<DeckViewCard *> &cardList = start->getCards();
		for (int j = 0; j < cardList.size(); ++j)
			if (cardList[j]->getName() == m->getCardName()) {
				card = cardList[j];
				break;
			}
		if (!card)
			continue;
		
		start->removeCard(card);
		target->addCard(card);
		card->setParentItem(target);
	}
}

void DeckViewScene::rearrangeItems()
{
	const int spacing = CARD_HEIGHT / 3;
	QList<DeckViewCardContainer *> contList = cardContainers.values();
	qreal totalHeight = -spacing;
	qreal totalWidth = 0;
	for (int i = 0; i < contList.size(); ++i) {
		DeckViewCardContainer *c = contList[i];
		c->rearrangeItems();
		c->setPos(0, totalHeight + spacing);
		totalHeight += c->boundingRect().height() + spacing;
		if (c->boundingRect().width() > totalWidth)
			totalWidth = c->boundingRect().width();
	}
	for (int i = 0; i < contList.size(); ++i)
		contList[i]->setWidth(totalWidth);
	
	setSceneRect(QRectF(0, 0, totalWidth, totalHeight));
}

void DeckViewScene::updateContents()
{
	rearrangeItems();
	emit sideboardPlanChanged();
}

QList<MoveCardToZone *> DeckViewScene::getSideboardPlan() const
{
	QList<MoveCardToZone *> result;
	QMapIterator<QString, DeckViewCardContainer *> containerIterator(cardContainers);
	while (containerIterator.hasNext()) {
		DeckViewCardContainer *cont = containerIterator.next().value();
		const QList<DeckViewCard *> cardList = cont->getCards();
		for (int i = 0; i < cardList.size(); ++i)
			if (cardList[i]->getOriginZone() != cont->getName())
				result.append(new MoveCardToZone(cardList[i]->getName(), cardList[i]->getOriginZone(), cont->getName()));
	}
	return result;
}

DeckView::DeckView(QWidget *parent)
	: QGraphicsView(parent)
{
	deckViewScene = new DeckViewScene(this);
	
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setDragMode(RubberBandDrag);
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
	setScene(deckViewScene);

	connect(deckViewScene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));
	connect(deckViewScene, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
	connect(deckViewScene, SIGNAL(sideboardPlanChanged()), this, SIGNAL(sideboardPlanChanged()));
}

void DeckView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	updateSceneRect(scene()->sceneRect());
}

void DeckView::updateSceneRect(const QRectF &rect)
{
	fitInView(rect, Qt::KeepAspectRatio);
}

void DeckView::setDeck(DeckList *_deck)
{
	deckViewScene->setDeck(_deck);
}
