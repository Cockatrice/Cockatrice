#include <QtGui>
#include "cardzone.h"
#include "carditem.h"
#include "player.h"
#include "zoneviewzone.h"
#include "protocol_items.h"

CardZone::CardZone(Player *_p, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent, bool isView)
	: AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), view(NULL), menu(NULL), doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable)
{
	if (!isView)
		player->addZone(this);
}

CardZone::~CardZone()
{
	qDebug(QString("CardZone destructor: %1").arg(name).toLatin1());
	delete view;
	clearContents();
}

void CardZone::clearContents()
{
	for (int i = 0; i < cards.size(); i++)
		delete cards.at(i);
	cards.clear();
}

QString CardZone::getTranslatedName(bool hisOwn, GrammaticalCase gc) const
{
	QString ownerName = player->getName();
	if (name == "hand")
		switch (gc) {
			case CaseNominative: return hisOwn ? tr("his hand") : tr("%1's hand").arg(ownerName);
			case CaseGenitive: return hisOwn ? tr("of his hand") : tr("of %1's hand").arg(ownerName);
			case CaseAccusative: return hisOwn ? tr("his hand") : tr("%1's hand").arg(ownerName);
		}
	else if (name == "deck")
		switch (gc) {
			case CaseNominative: return hisOwn ? tr("his library") : tr("%1's library").arg(ownerName);
			case CaseGenitive: return hisOwn ? tr("of his library") : tr("of %1's library").arg(ownerName);
			case CaseAccusative: return hisOwn ? tr("his library") : tr("%1's library").arg(ownerName);
		}
	else if (name == "grave")
		switch (gc) {
			case CaseNominative: return hisOwn ? tr("his graveyard") : tr("%1's graveyard").arg(ownerName);
			case CaseGenitive: return hisOwn ? tr("of his graveyard") : tr("of %1's graveyard").arg(ownerName);
			case CaseAccusative: return hisOwn ? tr("his graveyard") : tr("%1's graveyard").arg(ownerName);
		}
	else if (name == "rfg")
		switch (gc) {
			case CaseNominative: return hisOwn ? tr("his exile") : tr("%1's exile").arg(ownerName);
			case CaseGenitive: return hisOwn ? tr("of his exile") : tr("of %1's exile").arg(ownerName);
			case CaseAccusative: return hisOwn ? tr("his exile") : tr("%1's exile").arg(ownerName);
		}
	else if (name == "sb")
		switch (gc) {
			case CaseNominative: return hisOwn ? tr("his sideboard") : tr("%1's sideboard").arg(ownerName);
			case CaseGenitive: return hisOwn ? tr("of his sideboard") : tr("of %1's sideboard").arg(ownerName);
			case CaseAccusative: return hisOwn ? tr("his sideboard") : tr("%1's sideboard").arg(ownerName);
		}
	return QString();
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
	if (doubleClickAction)
		doubleClickAction->trigger();
}

void CardZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		if (menu) {
			menu->exec(event->screenPos());
			event->accept();
		} else
			event->ignore();
	} else
		event->ignore();
}

void CardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	dumpObjectInfo();
	if (view)
		if ((x <= view->getCards().size()) || (view->getNumberCards() == -1))
			view->addCard(new CardItem(player, card->getName(), card->getId()), reorganize, x, y);

	addCardImpl(card, x, y);

	if (reorganize) {
		qDebug("------------ emitting");
		emit contentsChanged();
		reorganizeCards();
	}
}

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
	CardItem *c = cards.findCard(cardId, false);
	if (!c) {
		qDebug() << "CardZone::getCard: card id=" << cardId << "not found";
		return 0;
	}
	// If the card's id is -1, this zone is invisible,
	// so we need to give the card an id and a name as it comes out.
	// It can be assumed that in an invisible zone, all cards are equal.
	if ((c->getId() == -1) || (c->getName().isEmpty())) {
		c->setId(cardId);
		c->setName(cardName);
	}
	return c;
}

CardItem *CardZone::takeCard(int position, int cardId, const QString &cardName, bool /*canResize*/)
{
	if (position == -1) {
		// position == -1 means either that the zone is indexed by card id
		// or that it doesn't matter which card you take.
		for (int i = 0; i < cards.size(); ++i)
			if (cards[i]->getId() == cardId) {
				position = i;
				break;
			}
		if (position == -1)
			position = 0;
	}
	if (position >= cards.size())
		return 0;

	CardItem *c = cards.takeAt(position);

	if (view)
		view->removeCard(position);

	c->setId(cardId);
	c->setName(cardName);

	emit contentsChanged();
	reorganizeCards();
	return c;
}

void CardZone::setCardAttr(int cardId, const QString &aname, const QString &avalue)
{
	if (hasCardAttr)
		player->sendGameCommand(new Command_SetCardAttr(-1, name, cardId, aname, avalue));
}

void CardZone::moveAllToZone()
{
	QList<QVariant> data = static_cast<QAction *>(sender())->data().toList();
	QString targetZone = data[0].toString();
	int targetX = data[1].toInt();

	// Cards need to be moved in reverse order so that the other
	// cards' list index doesn't change
	for (int i = cards.size() - 1; i >= 0; i--)
		player->sendGameCommand(new Command_MoveCard(-1, getName(), cards.at(i)->getId(), getName(), targetX));
}

QPointF CardZone::closestGridPoint(const QPointF &point)
{
	return point;
}
