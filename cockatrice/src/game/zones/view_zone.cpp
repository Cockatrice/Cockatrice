#include "view_zone.h"

#include "../../server/pending_command.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../cards/card_info.h"
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

/**
 * @param _p the player that the cards are revealed to.
 * @param _origZone the zone the cards were revealed from.
 * @param _revealZone if false, the cards will be face down.
 * @param _writeableRevealZone whether the player can interact with the revealed cards.
 * @param parent the parent QGraphicsWidget containing the reveal zone
 */
ZoneViewZone::ZoneViewZone(Player *_p,
                           CardZone *_origZone,
                           int _numberCards,
                           bool _revealZone,
                           bool _writeableRevealZone,
                           QGraphicsItem *parent,
                           bool _isReversed)
    : SelectZone(_p, _origZone->getName(), false, false, true, parent), bRect(QRectF()), minRows(0),
      numberCards(_numberCards), origZone(_origZone), revealZone(_revealZone),
      writeableRevealZone(_writeableRevealZone), groupBy(CardList::NoSort), sortBy(CardList::NoSort),
      isReversed(_isReversed)
{
    if (!(revealZone && !writeableRevealZone)) {
        origZone->getViews().append(this);
    }
}

/**
 * Deletes this ZoneView and removes it from the origZone's views.
 * You should normally call this method instead of deleteLater()
 */
void ZoneViewZone::close()
{
    emit closed();
    if (!(revealZone && !writeableRevealZone)) {
        origZone->getViews().removeOne(this);
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
    if (!cardList.isEmpty()) {
        for (int i = 0; i < cardList.size(); ++i) {
            auto card = cardList[i];
            CardRef cardRef = {QString::fromStdString(card->name()), QString::fromStdString(card->provider_id())};
            addCard(new CardItem(player, this, cardRef, card->id()), false, i);
        }
        reorganizeCards();
    } else if (!origZone->contentsKnown()) {
        Command_DumpZone cmd;
        cmd.set_player_id(player->getId());
        cmd.set_zone_name(name.toStdString());
        cmd.set_number_cards(numberCards);
        cmd.set_is_reversed(isReversed);

        PendingCommand *pend = player->prepareGameCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &ZoneViewZone::zoneDumpReceived);
        player->sendGameCommand(pend);
    } else {
        const CardList &c = origZone->getCards();
        int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
        for (int i = 0; i < number; i++) {
            CardItem *card = c.at(i);
            addCard(new CardItem(player, this, card->getCardRef(), card->getId()), false, i);
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
        auto *card = new CardItem(player, this, {cardName, cardProviderId}, cardInfo.id(), this);
        cards.insert(i, card);
    }

    updateCardIds(INITIALIZE);
    reorganizeCards();
    emit cardCountChanged();
}

void ZoneViewZone::updateCardIds(CardAction action)
{
    if (origZone->contentsKnown()) {
        return;
    }

    if (cards.isEmpty()) {
        return;
    }

    int cardCount = cards.size();

    auto startId = 0;

    if (isReversed) {
        // the card has not been added to origZone's cardList at this point
        startId = origZone->getCards().size() - cardCount;
        switch (action) {
            case INITIALIZE:
                break;
            case ADD_CARD:
                startId += 1;
                break;
            case REMOVE_CARD:
                startId -= 1;
                break;
        }
    }

    for (int i = 0; i < cardCount; ++i) {
        cards[i]->setId(i + startId);
    }
}

// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
    // filter cards
    CardList cardsToDisplay = CardList(cards.getContentsKnown());
    for (auto card : cards) {
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

/**
 * Checks if inserting a card at the given position requires an actual new card to be created and added to the view.
 * Also does any cardId updates that would be required if a card is inserted in that position.
 *
 * Note that this method can end up modifying the cardIds despite returning false.
 * (for example, if the card is inserted into a hidden portion of the deck while the view is reversed)
 *
 * Make sure to call this method once before calling addCard(), so that you skip creating a new CardItem and calling
 * addCard() if it's not required.
 *
 * @param x The position to insert the card at.
 * @return Whether to proceed with calling addCard.
 */
bool ZoneViewZone::prepareAddCard(int x)
{
    bool doInsert = false;
    if (!isReversed) {
        if (x <= cards.size() || cards.size() == -1) {
            doInsert = true;
        }
    } else {
        // map x (which is in origZone indexes) to this viewZone's cardList index
        int firstId = cards.isEmpty() ? origZone->getCards().size() : cards.front()->getId();
        int insertionIndex = x - firstId;
        if (insertionIndex >= 0) {
            // card was put into a portion of the deck that's in the view
            doInsert = true;
        } else {
            // card was put into a portion of the deck that's not in the view; update ids but don't insert card
            updateCardIds(ADD_CARD);
        }
    }

    // autoclose check is done both here and in removeCard
    if (cards.isEmpty() && !doInsert && SettingsCache::instance().getCloseEmptyCardView()) {
        close();
    }

    return doInsert;
}

/**
 * Make sure prepareAddCard() was called before calling addCard().
 * This method assumes we already checked that the card is being inserted into the visible portion
 */
void ZoneViewZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    if (!isReversed) {
        // if x is negative set it to add at end
        // if x is out-of-bounds then also set it to add at the end
        if (x < 0 || x >= cards.size()) {
            x = cards.size();
        }
        cards.insert(x, card);
    } else {
        // map x (which is in origZone indexes) to this viewZone's cardList index
        int firstId = cards.isEmpty() ? origZone->getCards().size() : cards.front()->getId();
        int insertionIndex = x - firstId;
        // qMin to prevent out-of-bounds error when bottoming a card that is already in the view
        cards.insert(qMin(insertionIndex, cards.size()), card);
    }

    card->setParentItem(this);
    card->update();

    updateCardIds(ADD_CARD);
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
    cmd.set_is_reversed(isReversed);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    player->sendGameCommand(cmd);
}

void ZoneViewZone::removeCard(int position, bool toNewZone)
{
    if (isReversed) {
        position -= cards.first()->getId();
        if (position < 0 || position >= cards.size()) {
            updateCardIds(REMOVE_CARD);
            return;
        }
    }

    if (position >= cards.size()) {
        return;
    }

    CardItem *card = cards.takeAt(position);
    card->deleteLater();

    // The toNewZone check is to prevent the view from auto-closing if the view contains only a single card and that
    // card gets dragged within the view.
    // Another autoclose check is done in prepareAddCard so that the view autocloses if the last card was moved to an
    // unrevealed portion of the same zone.
    if (cards.isEmpty() && SettingsCache::instance().getCloseEmptyCardView() && toNewZone) {
        close();
        return;
    }

    updateCardIds(REMOVE_CARD);
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
