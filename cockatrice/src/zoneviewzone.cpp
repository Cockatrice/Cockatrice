#include <QtGui>
#include "zoneviewzone.h"
#include "player.h"
#include "client.h"
#include "protocol_items.h"

ZoneViewZone::ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards, QGraphicsItem *parent)
	: CardZone(_p, _origZone->getName(), false, false, true, parent, true), height(0), numberCards(_numberCards), origZone(_origZone), sortingEnabled(false)
{
	origZone->setView(this);
}

ZoneViewZone::~ZoneViewZone()
{
	qDebug("ZoneViewZone destructor");
	origZone->setView(NULL);
}

QRectF ZoneViewZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH * 1.75, height);
}

void ZoneViewZone::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
}

void ZoneViewZone::initializeCards()
{
	if (!origZone->contentsKnown()) {
//		PendingCommand_DumpZone *dumpZoneCommand = player->client->dumpZone(player->getId(), name, numberCards);
//		connect(dumpZoneCommand, SIGNAL(cardListReceived(QList<ServerZoneCard>)), this, SLOT(zoneDumpReceived(QList<ServerZoneCard>)));
	} else {
		const CardList &c = origZone->getCards();
		int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
		for (int i = 0; i < number; i++) {
			CardItem *card = c.at(i);
			addCard(new CardItem(player, card->getName(), card->getId(), this), false, i);
		}
		emit contentsChanged();
		reorganizeCards();
	}
}

/*void ZoneViewZone::zoneDumpReceived(QList<ServerInfo_Card *> cards)
{
	for (int i = 0; i < cards.size(); i++) {
		CardItem *card = new CardItem(player, cards[i].getName(), i, this);
		addCard(card, false, i);
	}
	
	emit contentsChanged();
	reorganizeCards();
}
*/
// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
	qDebug("reorganizeCards");

	if (cards.isEmpty())
		return;

	int cardCount = cards.size();
	if (!origZone->contentsKnown())
		for (int i = 0; i < cardCount; ++i)
			cards[i]->setId(i);
	
	qreal totalWidth = boundingRect().width();
	qreal totalHeight = boundingRect().height();
	qreal cardWidth = cards.at(0)->boundingRect().width();
	qreal cardHeight = cards.at(0)->boundingRect().height();
	qreal x1 = 0;
	qreal x2 = (totalWidth - cardWidth);
	
	CardList cardsToDisplay(cards);
	if (sortingEnabled)
		cardsToDisplay.sort();
	
	for (int i = 0; i < cardCount; i++) {
		CardItem *c = cardsToDisplay.at(i);
		qreal x = i % 2 ? x2 : x1;
		// If the total height of the cards is smaller than the available height,
		// the cards do not need to overlap and are displayed in the center of the area.
		if (cardHeight * cardCount > totalHeight)
			c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
		else
			c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
		c->setZValue(i);
	}
}

void ZoneViewZone::setSortingEnabled(int _sortingEnabled)
{
	sortingEnabled = _sortingEnabled;
	reorganizeCards();
}

void ZoneViewZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards.insert(x, card);
	card->setParentItem(this);
	card->update();
}

void ZoneViewZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	qDebug(QString("handleDropEvent id=%1").arg(cardId).toLatin1());
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), 0, 0, false));
}

void ZoneViewZone::removeCard(int position)
{
	if (position >= cards.size())
		return;

	CardItem *card = cards.takeAt(position);
	delete card;
	reorganizeCards();
}

void ZoneViewZone::setGeometry(const QRectF &rect)
{
	setPos(rect.topLeft());
	height = rect.height();
	reorganizeCards();
}

QSizeF ZoneViewZone::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
	switch (which) {
		case Qt::MinimumSize: return QSizeF(1.75 * CARD_WIDTH, 2 * CARD_HEIGHT);
		case Qt::PreferredSize: return QSizeF(1.75 * CARD_WIDTH, constraint.height());
		case Qt::MaximumSize: return QSizeF(1.75 * CARD_WIDTH, constraint.height());
		default: return QSizeF();
	}
}
