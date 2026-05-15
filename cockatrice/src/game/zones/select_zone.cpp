#include "select_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../board/card_item.h"
#include "../game_scene.h"

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

static qreal stackingOffset(qreal cardHeight)
{
    const qreal overlapPercent = SettingsCache::instance().getStackCardOverlapPercent();
    return cardHeight * (100.0 - overlapPercent) / 100.0;
}

SelectZone::ZoneLayout SelectZone::computeZoneLayout(const StackLayoutParams &params)
{
    if (params.cardCount <= 0) {
        return {0.0, 0.0};
    }
    qreal effectiveOffset = params.desiredOffset;
    if (params.cardCount > 1) {
        qreal fitOffset;
        if (params.totalHeight < params.cardHeight && params.minOffset > 0.0) {
            // Zone is shorter than a card (e.g. minimized). Compress offsets so
            // every card has at least minOffset pixels of its top visible.
            fitOffset = (params.totalHeight - params.minOffset) / (params.cardCount - 1);
            effectiveOffset = qMax(0.0, qMin(params.desiredOffset, fitOffset));
        } else {
            qreal reservedForBottomCard;
            if (params.allowBottomOverflow) {
                // Allow the bottom card to partially overflow in tight zones, scaling the
                // overflow allowance by sqrt(cardCount-1) so offsets decrease smoothly
                // as cards are added rather than dropping by 1/(n-1) each time.
                // The 0.75 ratio was tuned experimentally to balance card visibility vs. overflow.
                constexpr qreal bottomCardZoneRatio = 0.75;
                const qreal adjustedRatio = bottomCardZoneRatio / qSqrt(static_cast<qreal>(params.cardCount - 1));
                reservedForBottomCard = qMin(params.cardHeight, params.totalHeight * adjustedRatio);
            } else {
                // No overflow: reserve full card height for the bottom card
                reservedForBottomCard = params.cardHeight;
            }
            fitOffset = (params.totalHeight - reservedForBottomCard) / (params.cardCount - 1);
            effectiveOffset = qMax(params.minOffset, qMin(params.desiredOffset, fitOffset));
        }
    }
    qreal stackHeight = (params.cardCount - 1) * effectiveOffset + params.cardHeight;
    qreal start = (stackHeight <= params.totalHeight) ? (params.totalHeight - stackHeight) / 2.0 : 0.0;
    return {effectiveOffset, start};
}

SelectZone *SelectZone::findOwningSelectZone(const QGraphicsItem *card)
{
    QGraphicsItem *parent = card ? card->parentItem() : nullptr;
    if (!parent) {
        return nullptr;
    }
    // Card may be direct child of zone (escaped for hover) or child of clip container.
    if (auto *zone = dynamic_cast<SelectZone *>(parent)) {
        return zone;
    }
    if (auto *zone = dynamic_cast<SelectZone *>(parent->parentItem())) {
        return zone;
    }
    return nullptr;
}

SelectZone::StackLayoutParams SelectZone::buildStackParams(qreal minOffset) const
{
    const auto &cards = getLogic()->getCards();
    if (cards.isEmpty())
        return {0, boundingRect().height(), 0.0, 0.0, minOffset};
    const auto cardCount = static_cast<int>(cards.size());
    const qreal cardHeight = cards.at(0)->boundingRect().height();
    const qreal offset = stackingOffset(cardHeight);
    return {cardCount, boundingRect().height(), cardHeight, offset, minOffset};
}

int SelectZone::calcDropIndexFromY(qreal dropY, qreal minOffset) const
{
    const auto &cards = getLogic()->getCards();
    if (cards.isEmpty()) {
        return 0;
    }
    const auto params = buildStackParams(minOffset);
    auto [effectiveOffset, start] = computeZoneLayout(params);
    if (effectiveOffset <= 0.0) {
        return 0;
    }
    return qBound(0, qRound((dropY - start) / effectiveOffset), params.cardCount - 1);
}

void SelectZone::restoreStaleEscapedCards()
{
    if (!cardClipContainer)
        return;
    for (auto *card : getLogic()->getCards()) {
        // A card parented to the zone (instead of the clip container) should
        // only occur while it is actively hovered. If hover cleanup was
        // missed, reparent it back so clipping resumes.
        if (card && card->parentItem() == this && !card->getIsHovered()) {
            card->setParentItem(cardClipContainer);
        }
    }
}

void SelectZone::layoutCardsVertically(const StackLayoutParams &params)
{
    const auto &cards = getLogic()->getCards();
    if (cards.isEmpty() || params.cardCount <= 0)
        return;
    if (params.cardCount > cards.size())
        return;

    constexpr qreal xspace = 5;
    const qreal cardWidth = cards.at(0)->boundingRect().width();
    const qreal totalWidth = boundingRect().width();
    const qreal x1 = xspace;
    const qreal x2 = totalWidth - xspace - cardWidth;
    const qreal xCentered = (totalWidth - cardWidth) / 2.0;

    auto [effectiveOffset, start] = computeZoneLayout(params);
    for (int i = 0; i < params.cardCount; i++) {
        CardItem *card = cards.at(i);
        qreal y = start + i * effectiveOffset;
        // Center single card; alternate left/right for multiple cards
        qreal x = (params.cardCount == 1) ? xCentered : ((i % 2) ? x2 : x1);
        card->setPos(x, y);
        card->setRealZValue(i);
    }
}

SelectZone::SelectZone(CardZoneLogic *_logic, QGraphicsItem *parent) : CardZone(_logic, parent)
{
}

SelectZone::~SelectZone()
{
    if (cardClipContainer) {
        // Reparent any hover-escaped cards back to the clip container so Qt's
        // parent-child tree is consistent for destruction. setParentItem() does
        // not invalidate getLogic()->getCards() (it modifies the graphics tree,
        // not the zone's logical card list).
        for (auto *card : getLogic()->getCards()) {
            if (card && card->parentItem() == this) {
                card->setParentItem(cardClipContainer);
            }
        }
    }
}

void SelectZone::onCardAdded(CardItem *addedCard)
{
    if (cardClipContainer && addedCard) {
        addedCard->setParentItem(cardClipContainer);
        addedCard->setVisible(true);
        addedCard->update();
    } else {
        CardZone::onCardAdded(addedCard);
    }
}

void SelectZone::setupClipContainer(std::optional<qreal> zValue)
{
    if (cardClipContainer)
        return;

    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);

    cardClipContainer = new QGraphicsRectItem(this); // Owned by Qt parent-child tree; deleted with this zone.
    cardClipContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    cardClipContainer->setPen(Qt::NoPen);
    cardClipContainer->setBrush(Qt::NoBrush);
    cardClipContainer->setRect(boundingRect());
    if (zValue.has_value()) {
        cardClipContainer->setZValue(*zValue);
    }
}

void SelectZone::escapeClipForHover(QGraphicsItem *card)
{
    // Reparent from clip container to zone so the hover-scaled card is visible
    // beyond clip bounds. Coordinates are identical because the clip container
    // is at (0,0) with no transform relative to this zone.
    if (cardClipContainer && card && card->parentItem() == cardClipContainer) {
        card->setParentItem(this);
        cardClipContainer->update();
    }
}

void SelectZone::restoreClipAfterHover(QGraphicsItem *card)
{
    // Restore card to clip container. If card's parent is not this zone,
    // a zone transition already reparented it via onCardAdded — skip.
    if (cardClipContainer && card && card->parentItem() == this) {
        card->setParentItem(cardClipContainer);
    }
}

void SelectZone::updateClipRect()
{
    if (cardClipContainer) {
        cardClipContainer->setRect(boundingRect());
    }
}

void SelectZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        QPointF pos = event->pos();
        if (pos.x() < 0)
            pos.setX(0);
        QRectF br = boundingRect();
        if (pos.x() > br.width())
            pos.setX(br.width());
        if (pos.y() < 0)
            pos.setY(0);
        if (pos.y() > br.height())
            pos.setY(br.height());

        QRectF selectionRect = QRectF(selectionOrigin, pos).normalized();
        for (auto card : getLogic()->getCards()) {
            if (card->getAttachedTo() && card->getAttachedTo()->getZone() != getLogic()) {
                continue;
            }

            bool inRect = selectionRect.intersects(card->mapRectToItem(this, card->boundingRect()));
            if (inRect && !cardsInSelectionRect.contains(card)) {
                // selection has just expanded to cover the card
                cardsInSelectionRect.insert(card);
                card->setSelected(!card->isSelected());
            } else if (!inRect && cardsInSelectionRect.contains(card)) {
                // selection has just shrunk to no longer cover the card
                cardsInSelectionRect.remove(card);
                card->setSelected(!card->isSelected());
            }
        }
        static_cast<GameScene *>(scene())->resizeRubberBand(
            deviceTransform(static_cast<GameScene *>(scene())->getViewportTransform()).map(pos),
            cardsInSelectionRect.size());
        event->accept();
    }
}

void SelectZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!event->modifiers().testFlag(Qt::ControlModifier)) {
            scene()->clearSelection();
        }

        selectionOrigin = event->pos();
        static_cast<GameScene *>(scene())->startRubberBand(event->scenePos());
        event->accept();
    } else
        CardZone::mousePressEvent(event);
}

void SelectZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    selectionOrigin = QPoint();
    cardsInSelectionRect.clear();
    static_cast<GameScene *>(scene())->stopRubberBand();
    event->accept();
}
