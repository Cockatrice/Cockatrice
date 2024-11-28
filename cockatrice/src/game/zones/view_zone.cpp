#include "view_zone.h"

#include "../../server/pending_command.h"
#include "../cards/card_database.h"
#include "../cards/card_drag_item.h"
#include "../cards/card_item.h"
#include "../player/player.h"
#include "pb/command_dump_zone.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/response_dump_zone.pb.h"
#include "pb/serverinfo_card.pb.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsSceneWheelEvent>
#include <QPainter>
#include <QtMath>

ZoneViewZone::ZoneViewZone(Player *_p,
                           CardZone *_origZone,
                           int _numberCards,
                           bool _revealZone,
                           bool _writeableRevealZone,
                           QGraphicsItem *parent)
    : SelectZone(_p, _origZone->getName(), false, false, true, parent, true), bRect(QRectF()), minRows(0),
      numberCards(_numberCards), origZone(_origZone), revealZone(_revealZone),
      writeableRevealZone(_writeableRevealZone), sortByName(false), sortByType(false)
{
    if (!(revealZone && !writeableRevealZone)) {
        origZone->getViews().append(this);
    }
}

ZoneViewZone::~ZoneViewZone()
{
    emit beingDeleted();
    qDebug("ZoneViewZone destructor");
    if (!(revealZone && !writeableRevealZone)) {
        origZone->getViews().removeOne(this);
    }
}

QRectF ZoneViewZone::boundingRect() const
{
    return bRect;
}

void ZoneViewZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush windowBrush(QColor(240, 240, 240));
    windowBrush.setColor(windowBrush.color().darker(150));
    painter->fillRect(boundingRect(), windowBrush);
}

void ZoneViewZone::initializeCards(const QList<const ServerInfo_Card *> &cardList)
{
    if (!cardList.isEmpty()) {
        for (int i = 0; i < cardList.size(); ++i)
            addCard(new CardItem(player, this, QString::fromStdString(cardList[i]->name()),
                                 QString::fromStdString(cardList[i]->provider_id()), cardList[i]->id(), revealZone),
                    false, i);
        reorganizeCards();
    } else if (!origZone->contentsKnown()) {
        Command_DumpZone cmd;
        cmd.set_player_id(player->getId());
        cmd.set_zone_name(name.toStdString());
        cmd.set_number_cards(numberCards);

        PendingCommand *pend = player->prepareGameCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(zoneDumpReceived(const Response &)));
        player->sendGameCommand(pend);
    } else {
        const CardList &c = origZone->getCards();
        int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
        for (int i = 0; i < number; i++) {
            CardItem *card = c.at(i);
            addCard(new CardItem(player, this, card->getName(), card->getProviderId(), card->getId(), revealZone),
                    false, i);
        }
        reorganizeCards();
    }
}

void ZoneViewZone::zoneDumpReceived(const Response &r)
{
    const Response_DumpZone &resp = r.GetExtension(Response_DumpZone::ext);
    const int respCardListSize = resp.zone_info().card_list_size();
    for (int i = 0; i < respCardListSize; ++i) {
        const ServerInfo_Card &cardInfo = resp.zone_info().card_list(i);
        auto cardName = QString::fromStdString(cardInfo.name());
        auto cardProviderId = QString::fromStdString(cardInfo.provider_id());
        auto *card = new CardItem(player, this, cardName, cardProviderId, cardInfo.id(), revealZone, this);
        cards.insert(i, card);
    }
    reorganizeCards();
    emit cardCountChanged();
}

// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
    int cardCount = cards.size();
    if (!origZone->contentsKnown())
        for (int i = 0; i < cardCount; ++i)
            cards[i]->setId(i);

    CardList cardsToDisplay(cards);
    if (sortByName || sortByType)
        cardsToDisplay.sort((sortByName ? CardList::SortByName : 0) | (sortByType ? CardList::SortByType : 0));

    auto gridSize = positionCardsForDisplay(cardsToDisplay, pileView && sortByType);

    qreal aleft = 0;
    qreal atop = 0;
    qreal awidth = gridSize.cols * CARD_WIDTH + (CARD_WIDTH / 2) + HORIZONTAL_PADDING;
    qreal aheight = (gridSize.rows * CARD_HEIGHT) / 3 + CARD_HEIGHT * 1.3;
    optimumRect = QRectF(aleft, atop, awidth, aheight);

    updateGeometry();
    emit optimumRectChanged();
}

/**
 * @brief Sets the position of each card to the proper position for the view
 *
 * @param cards The cards to reposition. Will modify the cards in the list.
 * @param groupByType Whether to make piles by card type. If true, then expects `cards` to be sorted by type.
 *
 * @returns The number of rows and columns to display
 */
ZoneViewZone::GridSize ZoneViewZone::positionCardsForDisplay(CardList &cards, bool groupByType)
{
    int cardCount = cards.size();
    if (groupByType) {
        int row = 0;
        int col = 0;
        int longestRow = 0;
        QString lastCardType;
        for (int i = 0; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            QString cardType = c->getInfo() ? c->getInfo()->getMainCardType() : "";

            if (i) { // if not the first card
                if (cardType == lastCardType)
                    row++; // add below current card
                else {     // if no match then move card to next column
                    col++;
                    row = 0;
                }
            }

            lastCardType = cardType;
            qreal x = col * CARD_WIDTH;
            qreal y = row * CARD_HEIGHT / 3;
            c->setPos(HORIZONTAL_PADDING + x, VERTICAL_PADDING + y);
            c->setRealZValue(i);
            longestRow = qMax(row, longestRow);
        }

        return GridSize{longestRow, qMax(col + 1, 3)};

    } else {
        int cols = qFloor(qSqrt((double)cardCount / 2));
        if (cols > 7)
            cols = 7;
        int rows = qCeil((double)cardCount / cols);
        if (rows < 1)
            rows = 1;
        if (minRows == 0)
            minRows = rows;
        else if (rows < minRows) {
            rows = minRows;
            cols = qCeil((double)cardCount / minRows);
        }
        if (cols < 2)
            cols = 2;

        qDebug() << "reorganizeCards: rows=" << rows << "cols=" << cols;

        for (int i = 0; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            qreal x = (i / rows) * CARD_WIDTH;
            qreal y = (i % rows) * CARD_HEIGHT / 3;
            c->setPos(HORIZONTAL_PADDING + x, VERTICAL_PADDING + y);
            c->setRealZValue(i);
        }

        return GridSize{rows, qMax(cols, 1)};
    }
}

void ZoneViewZone::setSortByName(int _sortByName)
{
    sortByName = _sortByName;
    reorganizeCards();
}

void ZoneViewZone::setSortByType(int _sortByType)
{
    sortByType = _sortByType;
    if (!sortByType)
        pileView = false;
    reorganizeCards();
}

void ZoneViewZone::setPileView(int _pileView)
{
    pileView = _pileView;
    reorganizeCards();
}

void ZoneViewZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    // if x is negative set it to add at end
    if (x < 0 || x >= cards.size()) {
        x = cards.size();
    }
    cards.insert(x, card);
    card->setParentItem(this);
    card->update();
    reorganizeCards();
}

void ZoneViewZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                   CardZone *startZone,
                                   const QPoint & /*dropPoint*/)
{
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    player->sendGameCommand(cmd);
}

void ZoneViewZone::removeCard(int position)
{
    if (position >= cards.size())
        return;

    CardItem *card = cards.takeAt(position);
    card->deleteLater();
    reorganizeCards();
}

void ZoneViewZone::setGeometry(const QRectF &rect)
{
    prepareGeometryChange();
    setPos(rect.x(), rect.y());
    bRect = QRectF(0, 0, rect.width(), rect.height());
}

QSizeF ZoneViewZone::sizeHint(Qt::SizeHint /*which*/, const QSizeF & /*constraint*/) const
{
    return optimumRect.size();
}

void ZoneViewZone::setWriteableRevealZone(bool _writeableRevealZone)
{
    if (writeableRevealZone && !_writeableRevealZone) {
        origZone->getViews().append(this);
    } else if (!writeableRevealZone && _writeableRevealZone) {
        origZone->getViews().removeOne(this);
    }
    writeableRevealZone = _writeableRevealZone;
}

void ZoneViewZone::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    emit wheelEventReceived(event);
}
