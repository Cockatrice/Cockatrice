#include <QtGui>
#include "deckview.h"
#include "decklist.h"
#include "carddatabase.h"
#include "main.h"
#include <QDebug>

DeckViewCard::DeckViewCard(const QString &_name, QGraphicsItem *parent)
	: AbstractCardItem(_name, parent)
{
}

DeckViewCardContainer::DeckViewCardContainer(const QString &_name)
	: QGraphicsItem(), name(_name), width(0), height(0)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/table").toString();
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
	painter->drawText(10, 0, width - 20, separatorY, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, InnerDecklistNode::visibleNameFromName(name));
}

void DeckViewCardContainer::addCard(DeckViewCard *card)
{
	cards.insertMulti(card->getInfo()->getMainCardType(), card);
}

void DeckViewCardContainer::rearrangeItems()
{
	separatorY = 30;
	
	QList<QString> cardTypeList = cards.uniqueKeys();
	int rows = cardTypeList.size();
	int cols = 0;
	for (int i = 0; i < rows; ++i) {
		QList<DeckViewCard *> row = cards.values(cardTypeList[i]);
		if (row.size() > cols)
			cols = row.size();
		for (int j = 0; j < row.size(); ++j) {
			DeckViewCard *card = row[j];
			card->setPos(j * CARD_WIDTH, separatorY + 10 + i * (CARD_HEIGHT + rowSpacing));
		}
	}
	
	prepareGeometryChange();
	width = cols * CARD_WIDTH;
	height = separatorY + 10 + rows * CARD_HEIGHT + rowSpacing * (rows - 1);
}

void DeckViewCardContainer::setWidth(qreal _width)
{
	prepareGeometryChange();
	width = _width;
	update();
}

DeckViewScene::DeckViewScene(QObject *parent)
	: QGraphicsScene(parent), deck(0)
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
				DeckViewCard *newCard = new DeckViewCard(currentCard->getName(), container);
				container->addCard(newCard);
				emit newCardAdded(newCard);
			}
		}
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

DeckView::DeckView(QWidget *parent)
	: QGraphicsView(parent)
{
	deckViewScene = new DeckViewScene(this);
	
	setBackgroundBrush(QBrush(QColor(0, 0, 0)));
	setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
	setScene(deckViewScene);

	connect(deckViewScene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));
	connect(deckViewScene, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
}

void DeckView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	updateSceneRect(scene()->sceneRect());
}

void DeckView::updateSceneRect(const QRectF &rect)
{
	qDebug(QString("deckView::updateSceneRect = %1,%2").arg(rect.width()).arg(rect.height()).toLatin1());
	fitInView(rect, Qt::KeepAspectRatio);
}

void DeckView::setDeck(DeckList *_deck)
{
	deckViewScene->setDeck(_deck);
}
