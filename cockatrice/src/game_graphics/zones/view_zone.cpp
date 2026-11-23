#include "view_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/theme_manager.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../card_dimensions.h"
#include "../z_values.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/filters/filter_string.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_zone.pb.h"
#include "libcockatrice/utility/card_ref.h"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QMessageLogger>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QSharedPointer>
#include <QtGlobal>
#include <QtMath>
#include <functional>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_info.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/game/zones/view_zone_logic.h>
#include <libcockatrice/protocol/pb/response_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <string>

class QGraphicsItem;

namespace
{
constexpr qreal MARKER_HORIZONTAL_PADDING = 4.0;
constexpr qreal MARKER_VERTICAL_PADDING = 2.0;
constexpr qreal MARKER_CORNER_RADIUS = 4.0;
constexpr qreal MARKER_INSET = 2.0;
constexpr int MARKER_FONT_SIZE = 9;
// Vertical pitch of a card column: each card shows this much before the next
// card overlaps it. Marker placement uses it to stay on the visible strip.
constexpr qreal CARD_STACK_OFFSET = CardDimensions::HEIGHT_F / 3.0;

QFont markerFont()
{
    QFont font = themeManager != nullptr ? themeManager->appFont(AppFont::CardText) : QFont();
    font.setPixelSize(MARKER_FONT_SIZE);
    font.setWeight(QFont::Bold);
    return font;
}
} // namespace

LibraryEdgeMarker::LibraryEdgeMarker(QGraphicsItem *parent) : QGraphicsItem(parent)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
}

void LibraryEdgeMarker::setLabel(const QString &newLabel, const QString &toolTip)
{
    if (label == newLabel && this->toolTip() == toolTip) {
        return;
    }

    prepareGeometryChange();
    label = newLabel;

    const QFontMetrics metrics(markerFont());
    markerRect = QRectF(0, 0, metrics.horizontalAdvance(label) + 2 * MARKER_HORIZONTAL_PADDING,
                        metrics.height() + 2 * MARKER_VERTICAL_PADDING);

    setToolTip(toolTip);
    update();
}

QRectF LibraryEdgeMarker::boundingRect() const
{
    return markerRect;
}

void LibraryEdgeMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (label.isEmpty()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(markerFont());

    // ToolTipBase/ToolTipText are the palette's designed background/text pair
    // for small overlays, so the chip keeps its contrast in every theme.
    const QPalette palette = qApp->palette();
    painter->setPen(QPen(palette.color(QPalette::Mid), 1));
    painter->setBrush(palette.color(QPalette::ToolTipBase));
    painter->drawRoundedRect(markerRect.adjusted(0.5, 0.5, -0.5, -0.5), MARKER_CORNER_RADIUS, MARKER_CORNER_RADIUS);

    painter->setPen(palette.color(QPalette::ToolTipText));
    painter->drawText(markerRect, Qt::AlignCenter, label);
    painter->restore();
}

/**
 * @param parent the parent QGraphicsWidget containing the reveal zone
 */
ZoneViewZone::ZoneViewZone(ZoneViewZoneLogic *_logic, QGraphicsItem *parent)
    : SelectZone(_logic, parent), bRect(QRectF()), minRows(0), groupBy(CardList::NoSort), sortBy(CardList::NoSort)
{
    libraryEdgeMarker = new LibraryEdgeMarker(this);
    libraryEdgeMarker->setVisible(false);

    auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    if (!(viewLogic->getRevealZone() && !viewLogic->getWriteableRevealZone())) {
        viewLogic->getOriginalZone()->getViews().append(viewLogic);
    }
    connect(_logic, &ZoneViewZoneLogic::closeView, this, &ZoneViewZone::close);
}

void ZoneViewZone::addToViews()
{
    auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    viewLogic->getOriginalZone()->getViews().append(viewLogic);
}

void ZoneViewZone::removeFromViews()
{
    auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    viewLogic->getOriginalZone()->getViews().removeOne(viewLogic);
}

/**
 * Deletes this ZoneView and removes it from the origZone's views.
 * You should normally call this method instead of deleteLater()
 */
void ZoneViewZone::close()
{
    emit closed();
    auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    if (!(viewLogic->getRevealZone() && !viewLogic->getWriteableRevealZone())) {
        viewLogic->getOriginalZone()->getViews().removeOne(viewLogic);
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
            auto copy = new CardItem(getLogic()->getPlayer(), this, cardRef, card->id());
            copy->setFaceDown(card->face_down());

            getLogic()->addCard(copy->getState(), false, i);
        }
        reorganizeCards();
    } else if (!qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->contentsKnown()) {
        auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
        PendingCommand *pend = viewLogic->prepareZoneDump();
        if (pend != nullptr) {
            // A local game answers synchronously, so the handler has to be connected before the
            // command is sent or the response would be delivered to nobody.
            connect(pend, &PendingCommand::finished, this, &ZoneViewZone::zoneDumpReceived);
            viewLogic->sendZoneDump(pend);
        }
    } else {
        const CardList &c = qobject_cast<ZoneViewZoneLogic *>(getLogic())->getOriginalZone()->getCards();
        int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
        for (int i = 0; i < number; i++) {
            CardState *card = c.at(i);
            auto copy = new CardItem(getLogic()->getPlayer(), this, card->getCardRef(), card->getId());
            copy->setFaceDown(card->getFaceDown());

            getLogic()->addCard(copy->getState(), false, i);
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
        card->setFaceDown(cardInfo.face_down());
        getLogic()->rawInsertCard(card->getState(), i);
    }

    qobject_cast<ZoneViewZoneLogic *>(getLogic())->updateCardIds(ZoneViewZoneLogic::INITIALIZE);
    reorganizeCards();
    // clang-format off
    emit getLogic()->cardCountChanged(); // emit keyword causes spurious spacing around ->
    // clang-format on
}

// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
    // filter cards
    CardList cardsToDisplay = CardList(getLogic()->contentsKnown());
    for (auto card : cardItems()) {
        if (filterString.check(card->getCard().getCardPtr())) {
            card->show();
            cardsToDisplay.append(card->getState());
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

    updateLibraryEdgeMarker(cardsToDisplay);

    // determine bounding rect
    qreal aleft = 0;
    qreal atop = 0;
    qreal awidth = gridSize.cols * CardDimensions::WIDTH_F + CardDimensions::WIDTH_HALF_F + HORIZONTAL_PADDING;
    qreal aheight = (gridSize.rows * CardDimensions::HEIGHT_F) / 3 + CardDimensions::HEIGHT_F * 1.3;
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
            CardState *c = cards.at(i);
            CardItem *cItem = qobject_cast<CardItem *>(c->parent());
            QString columnProp = extractor(c);

            if (i) { // if not the first card
                if (columnProp == lastColumnProp) {
                    row++; // add below current card
                } else {   // if no match then move card to next column
                    col++;
                    row = 0;
                }
            }

            lastColumnProp = columnProp;
            qreal x = col * CardDimensions::WIDTH_F;
            qreal y = row * CARD_STACK_OFFSET;
            cItem->setPos(HORIZONTAL_PADDING + x, VERTICAL_PADDING + y);
            cItem->setRealZValue(i);
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

        if (cols < 2) {
            cols = 2;
        }

        qCDebug(ViewZoneLog) << "reorganizeCards: rows=" << rows << "cols=" << cols;

        for (int i = 0; i < cardCount; i++) {
            auto *cItem = qobject_cast<CardItem *>(cards.at(i)->parent());
            qreal x = (i / rows) * CardDimensions::WIDTH_F;
            qreal y = (i % rows) * CARD_STACK_OFFSET;
            cItem->setPos(HORIZONTAL_PADDING + x, VERTICAL_PADDING + y);
            cItem->setRealZValue(i);
        }

        return GridSize{rows, qMax(cols, 1)};
    }
}

void ZoneViewZone::updateLibraryEdgeMarker(const CardList &displayedCards)
{
    auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    if (viewLogic == nullptr || !viewLogic->isLibraryEdgeView() || displayedCards.isEmpty()) {
        libraryEdgeMarkerAnchorPos = QPointF();
        libraryEdgeMarker->hide();
        return;
    }

    const bool topEdge = viewLogic->showsTopOfLibrary();
    CardState *edgeState = topEdge ? displayedCards.first() : displayedCards.last();
    auto *edgeCard = edgeState == nullptr ? nullptr : qobject_cast<CardItem *>(edgeState->parent());
    if (edgeCard == nullptr) {
        libraryEdgeMarkerAnchorPos = QPointF();
        libraryEdgeMarker->hide();
        return;
    }

    libraryEdgeMarkerAnchorPos = edgeCard->pos();
    applyLibraryEdgeMarkerText();
    positionLibraryEdgeMarker();

    // Above the hovered-card layer: the chip sits inside its card, so hovering
    // that card must not let the card repaint over the indicator.
    libraryEdgeMarker->setZValue(ZValues::HOVERED_CARD + 0.5);
    libraryEdgeMarker->show();
}

void ZoneViewZone::positionLibraryEdgeMarker()
{
    // The chip has to stay on the part of its card that is not covered by the
    // next card in the column. The top card only shows its top strip, and its
    // name is drawn top-left, so the chip goes to the right of that strip. The
    // bottom card is fully visible, so the chip goes to its bottom-left, away
    // from the bottom-right power/toughness.
    const auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    const QRectF markerRect = libraryEdgeMarker->boundingRect();
    const bool topEdge = viewLogic != nullptr && viewLogic->showsTopOfLibrary();
    const QPointF offset = topEdge
                               ? QPointF(CardDimensions::WIDTH_F - markerRect.width() - MARKER_INSET,
                                         CARD_STACK_OFFSET - markerRect.height() - MARKER_INSET)
                               : QPointF(MARKER_INSET, CardDimensions::HEIGHT_F - markerRect.height() - MARKER_INSET);
    libraryEdgeMarker->setPos(libraryEdgeMarkerAnchorPos + offset);
}

void ZoneViewZone::applyLibraryEdgeMarkerText()
{
    const auto *viewLogic = qobject_cast<ZoneViewZoneLogic *>(getLogic());
    if (viewLogic == nullptr) {
        return;
    }

    const bool topEdge = viewLogic->showsTopOfLibrary();
    //: Short badge on the card at the top or bottom of a viewed library. The full phrase is in the tooltip.
    libraryEdgeMarker->setLabel(topEdge ? tr("Top") : tr("Bottom"),
                                topEdge ? tr("This card is on top of the library")
                                        : tr("This card is at the bottom of the library"));
}

void ZoneViewZone::retranslateEdgeMarker()
{
    if (libraryEdgeMarker->isVisible()) {
        // The translated label can be a different width, so reposition the chip
        // to keep it inside its card.
        applyLibraryEdgeMarkerText();
        positionLibraryEdgeMarker();
    }
}

void ZoneViewZone::setFilterString(const QString &_filterString)
{
    const CardsDisplaySettings &cardsDisplay = SettingsCache::instance().cardsDisplay();
    filterString = FilterString(
        _filterString, CardSearchLanguage{cardsDisplay.getCardLang(),
                                          static_cast<SearchLanguageMode>(cardsDisplay.getCardSearchLanguage())});
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
    QList<CardMoveRequest> cards;
    cards.reserve(dragItems.size());
    for (int i = 0; i < dragItems.size(); ++i) {
        cards.append(CardMoveRequest{dragItems[i]->getId(), dragItems[i]->isForceFaceDown()});
    }

    getLogic()->getPlayer()->getPlayerActions()->moveCards(startZone, getLogic(), 0, 0, cards);
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
