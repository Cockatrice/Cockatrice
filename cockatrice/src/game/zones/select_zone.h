/**
 * @file select_zone.h
 * @ingroup GameGraphicsZones
 * @brief Base class for zones where cards are laid out and individually interactable.
 */

#ifndef SELECTZONE_H
#define SELECTZONE_H

#include "card_zone.h"

#include <QSet>
#include <optional>

class QGraphicsRectItem;

/**
 * A CardZone where the cards are laid out, with each card directly interactable by clicking.
 */
class SelectZone : public CardZone
{
    Q_OBJECT
public:
    /// Finds the SelectZone that owns a card, regardless of whether the card is parented
    /// to the zone directly or to its clip container. Returns nullptr if not in a SelectZone.
    static SelectZone *findOwningSelectZone(const QGraphicsItem *card);

    SelectZone(CardZoneLogic *logic, QGraphicsItem *parent = nullptr);
    ~SelectZone() override;
    void onCardAdded(CardItem *addedCard) override;

    /// @brief Temporarily reparents a card from the clip container to this zone so hover scaling is visible beyond clip
    /// bounds. Safe no-op if no clip container exists. Coordinates are preserved (clip container is at (0,0) with no
    /// transform).
    void escapeClipForHover(QGraphicsItem *card);
    /// @brief Restores a hover-escaped card back to the clip container. Guards against zone transitions that already
    /// reparented the card.
    void restoreClipAfterHover(QGraphicsItem *card);

private:
    QPointF selectionOrigin;
    QSet<CardItem *> cardsInSelectionRect;
    /// Invisible clipping parent for cards; owned by Qt parent-child tree (parented to this zone).
    /// Created by setupClipContainer(); null when no clip container is active.
    QGraphicsRectItem *cardClipContainer = nullptr;

protected:
    // -- Layout computation --

    /// Parameters describing a vertical card stack's geometry.
    struct StackLayoutParams
    {
        int cardCount;
        qreal totalHeight;
        qreal cardHeight;
        qreal desiredOffset;
        qreal minOffset = 0.0;
        /// When false (default), reserves full cardHeight for the bottom card, ensuring
        /// all cards remain within zone bounds. When true, allows the bottom card to
        /// partially overflow using sqrt-scaled allowance. Use with setupClipContainer()
        /// for zones too short to fit a full card.
        bool allowBottomOverflow = false;
    };

    /// Result of computing a vertical stack layout.
    struct ZoneLayout
    {
        qreal effectiveOffset; ///< Actual offset between card tops (may be compressed)
        qreal start;           ///< Y coordinate of the first card's top edge
    };

    /// Minimum visible pixels of each card's top edge when stacking compresses offsets in tight zones.
    static constexpr qreal MIN_CARD_VISIBLE = 10.0;

    /**
     * @brief Computes layout for a vertical card stack (effective offset and start position).
     *
     * Three regimes:
     *  1. Minimized zone (totalHeight < card height with minOffset > 0): offsets compress
     *     so each card retains at least minOffset visible pixels of its top edge.
     *  2. Normal zone with allowBottomOverflow=false (default): the bottom card is
     *     guaranteed to fit within the zone boundary. Offsets compress as needed.
     *  3. Normal zone with allowBottomOverflow=true: the bottom card may partially
     *     overflow. The overflow allowance is scaled by sqrt(cardCount-1) so that
     *     adding one card shifts existing cards smoothly.
     *
     * When the stack fits with room to spare, it is centered vertically.
     */
    static ZoneLayout computeZoneLayout(const StackLayoutParams &params);

    /// Builds StackLayoutParams from the current card list and zone geometry.
    StackLayoutParams buildStackParams(qreal minOffset = 0.0) const;

    /// Computes the card index at a given y-coordinate within the zone's vertical layout.
    /// Returns 0 if the zone has no cards or the offset is zero.
    int calcDropIndexFromY(qreal dropY, qreal minOffset = 0.0) const;

    /**
     * @brief Positions cards vertically with alternating left/right x-offsets.
     *
     * Cards alternate between left and right margins (5px padding from zone edges):
     * even-indexed cards at left, odd-indexed at right.
     * Cards are assigned ascending z-values.
     *
     * @param params Stack layout geometry parameters (use allowBottomOverflow to control overflow)
     */
    void layoutCardsVertically(const StackLayoutParams &params);

    // -- Clip container --
    // The clip container mechanism is available for future zones that need visual clipping
    // (e.g., zones too short to fit a full card). To enable: call setupClipContainer() in the
    // zone's constructor, and set allowBottomOverflow=true in layout params.

    /// Restores any cards that were hover-escaped but whose hover state was not properly cleaned up.
    /// Call at the start of reorganizeCards() in zones that use a clip container.
    void restoreStaleEscapedCards();

    /// Creates a clip container child item that clips card overflow to zone bounds.
    /// Cards entering this zone are reparented to this container by the onCardAdded override.
    /// Disables zone-level child clipping; clipping is delegated to the container.
    /// @param zValue Optional z-value for the clip container (e.g. ZValues::CARD_BASE)
    void setupClipContainer(std::optional<qreal> zValue = std::nullopt);

    /// Updates the clip container rect to match this zone's current boundingRect().
    void updateClipRect();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
