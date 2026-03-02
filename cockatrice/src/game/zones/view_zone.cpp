#include "view_zone.h"

#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "logic/view_zone_logic.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsSceneWheelEvent>
#include <QPainter>
#include <QtMath>
#include <libcockatrice/protocol/pb/command_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/response_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/protocol/pending_command.h>

/**
 * @param parent the parent QGraphicsWidget containing the reveal zone
 */
ZoneViewZone::ZoneViewZone(ZoneViewZoneLogic *_logic, QGraphicsItem *parent)
    : SelectZone(_logic, parent), bRect(QRectF()), minRows(0), groupBy(CardList::NoSort), sortBy(CardList::NoSort)
{
    if (!(qobject_cast<ZoneViewZoneLogic *>(getLogic())->getRevealZone() &&
          !qobject_cast<ZoneViewZoneLogic *>(getLogic())->getWriteableRevealZone())) {
        qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getViews().append(this);
    }
    connect(_logic, &ZoneViewZoneLogic::closeView, this, &ZoneViewZone::close);
}

void ZoneViewZone::addToViews()
{
    qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getViews().append(this);
}

void ZoneViewZone::removeFromViews()
{
    qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getViews().removeOne(this);
}

/**
 * Deletes this ZoneView and removes it from the origZone's views.
 * You should normally call this method instead of deleteLater()
 */
void ZoneViewZone::close()
{
    emit closed();
    if (!(qobject_cast<ZoneViewZoneLogic *>(getLogic())->getRevealZone() &&
          !qobject_cast<ZoneViewZoneLogic *>(getLogic())->getWriteableRevealZone())) {
        qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getViews().removeOne(this);
    }
    deleteLater();
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
    int numberCards = qobject_cast<ZoneViewZoneLogic *>(getLogic())->getNumberCards();
    if (!cardList.isEmpty()) {
        for (int i = 0; i < cardList.size(); ++i) {
            auto card = cardList[i];
            CardRef cardRef = {QString::fromStdString(card->name()), QString::fromStdString(card->provider_id())};
            getLogic()->addCard(new CardItem(getLogic()->getPlayer(), this, cardRef, card->id()), false, i);
        }
        reorganizeCards();
    } else if (!qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->contentsKnown()) {
        Command_DumpZone cmd;
        cmd.set_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
        cmd.set_zone_name(getLogic()->getName().toStdString());
        cmd.set_number_cards(numberCards);
        cmd.set_is_reversed(qobject_cast<ZoneViewZoneLogic *>(getLogic())->getIsReversed());

        PendingCommand *pend = getLogic()->getPlayer()->getPlayerActions()->prepareGameCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &ZoneViewZone::zoneDumpReceived);
        getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(pend);
    } else {
        const CardList &c = qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getCards();
        int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
        for (int i = 0; i < number; i++) {
            CardItem *card = c.at(i);
            getLogic()->addCard(new CardItem(getLogic()->getPlayer(), this, card->getCardRef(), card->getId()), false,
                                i);
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
        auto card = new CardItem(getLogic()->getPlayer(), this, {cardName, cardProviderId}, cardInfo.id(), getLogic());
        getLogic()->rawInsertCard(card, i);
    }

    qobject_cast<ZoneViewZoneLogic *>(getLogic())->updateCardIds(ZoneViewZoneLogic::INITIALIZE);
    reorganizeCards();
    emit getLogic()->cardCountChanged();
}

// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
    // filter cards
    CardList cardsToDisplay = CardList(getLogic()->getCards().getContentsKnown());
    for (auto card : getLogic()->getCards()) {
        if (filterString.check(card->getCard().getCardPtr())) {
            card->show();
            cardsToDisplay.append(card);
        } else {
            card->hide();
        }
    }

    // sort cards
    QList<CardList::SortOption> sortOptions;
    if (groupBy != CardList::NoSort) {
        sortOptions << groupBy;
    }

    if (sortBy != CardList::NoSort) {
        sortOptions << sortBy;

        // implicitly sort by name at the end so that cards with the same name appear together
        if (sortBy != CardList::SortByName) {
            sortOptions << CardList::SortByName;
        }

        // group printings together
        sortOptions << CardList::SortByPrinting;
    }

    cardsToDisplay.sortBy(sortOptions);

    // position cards
    GridSize gridSize;
    if (pileView) {
        gridSize = positionCardsForDisplay(cardsToDisplay, groupBy);
    } else {
        gridSize = positionCardsForDisplay(cardsToDisplay);
    }

    // determine bounding rect
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
 * @param pileOption Property used to group cards for the piles. Expects `cards` to be sorted by that property. Pass in
 * NoSort to not make piles.
 *
 * @returns The number of rows and columns to display
 */
ZoneViewZone::GridSize ZoneViewZone::positionCardsForDisplay(CardList &cards, CardList::SortOption pileOption)
{
    int cardCount = cards.size();

    if (pileOption != CardList::NoSort) {
        int row = 0;
        int col = 0;
        int longestRow = 0;

        QString lastColumnProp;

        const auto extractor = CardList::getExtractorFor(pileOption);

        for (int i = 0; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            QString columnProp = extractor(c);

            if (i) { // if not the first card
                if (columnProp == lastColumnProp)
                    row++; // add below current card
                else {     // if no match then move card to next column
                    col++;
                    row = 0;
                }
            }

            lastColumnProp = columnProp;
            qreal x = col * CARD_WIDTH;
            qreal y = row * CARD_HEIGHT / 3;
            c->setPos(HORIZONTAL_PADDING + x, VERTICAL_PADDING + y);
            c->setRealZValue(i);
            longestRow = qMax(row, longestRow);
        }

        // +1 because the row/col variables used in the calculations are 0-indexed but
        // GridSize expects the actual row/col count
        return GridSize{longestRow + 1, qMax(col + 1, 3)};

    } else {
        int cols = qBound(1, qFloor(qSqrt((double)cardCount / 2)), 7);
        int rows = qMax(qCeil((double)cardCount / cols), 1);
        if (minRows == 0) {
            minRows = rows;
        } else if (rows < minRows) {
            rows = minRows;
            cols = qCeil((double)cardCount / minRows);
        }

        if (cols < 2)
            cols = 2;

        qCDebug(ViewZoneLog) << "reorganizeCards: rows=" << rows << "cols=" << cols;

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

void ZoneViewZone::setFilterString(const QString &_filterString)
{
    filterString = FilterString(_filterString);
    reorganizeCards();
}

void ZoneViewZone::setGroupBy(CardList::SortOption _groupBy)
{
    groupBy = _groupBy;
    reorganizeCards();
}

void ZoneViewZone::setSortBy(CardList::SortOption _sortBy)
{
    sortBy = _sortBy;
    reorganizeCards();
}

void ZoneViewZone::setPileView(int _pileView)
{
    pileView = _pileView;
    reorganizeCards();
}

void ZoneViewZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                   CardZoneLogic *startZone,
                                   const QPoint & /*dropPoint*/)
{
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());
    cmd.set_x(0);
    cmd.set_y(0);
    cmd.set_is_reversed(qobject_cast<ZoneViewZoneLogic *>(getLogic())->getIsReversed());

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
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

void ZoneViewZone::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    emit wheelEventReceived(event);
}
