/**
 * @file player_graphics_item.h
 * @ingroup GameGraphicsPlayers
 * @brief Root QGraphicsObject container that owns and lays out all visual game elements for a single player.
 */

#ifndef COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#define COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#include "../game_scene.h"
#include "player.h"

#include <QGraphicsObject>

class CommandZone;
class HandZone;
class PileZone;
class PlayerTarget;
class StackZone;
class TableZone;
class ZoneViewZone;

/**
 * The top-level graphics item representing a single player's entire board state in the game scene.
 *
 * PlayerGraphicsItem owns and manages every zone graphics item belonging to one player:
 * the table (battlefield), hand, deck, graveyard, removed-from-game, sideboard, stack,
 * and command/partner zones. It also owns the player avatar target widget, the background
 * PlayerArea, and the hand card counter.
 *
 * Layout responsibilities include:
 * - Positioning all zones relative to one another based on hand orientation
 *   (horizontal or vertical) and whether the view is mirrored (opponent's perspective).
 * - Responding to scene width changes by resizing the table and hand zones proportionally.
 * - Managing the 55-pixel left counter strip where life totals and other counters are displayed.
 * - Showing or hiding command/partner zones when the server signals Commander format support.
 *
 * This item emits sizeChanged() whenever its bounding rect changes, which GameScene uses
 * to reflow the positions of all players in the scene.
 *
 * @see PlayerArea, PlayerTarget, TableZone, HandZone, PileZone, StackZone, CommandZone
 */
class PlayerGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    enum
    {
        Type = typeOther
    };
    int type() const override
    {
        return Type;
    }

    static constexpr int counterAreaWidth = 55;

    explicit PlayerGraphicsItem(Player *player);
    void initializeZones();

    [[nodiscard]] QRectF boundingRect() const override;
    qreal getMinimumWidth() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void processSceneSizeChange(int newPlayerWidth);

    void setMirrored(bool _mirrored);

    bool getMirrored() const
    {
        return mirrored;
    }

    GameScene *getGameScene() const
    {
        return static_cast<GameScene *>(scene());
    }

    Player *getPlayer() const
    {
        return player;
    }

    PlayerArea *getPlayerArea() const
    {
        return playerArea;
    }

    PlayerTarget *getPlayerTarget() const
    {
        return playerTarget;
    }

    [[nodiscard]] PileZone *getDeckZoneGraphicsItem() const
    {
        return deckZoneGraphicsItem;
    }

    [[nodiscard]] PileZone *getSideboardZoneGraphicsItem() const
    {
        return sideboardGraphicsItem;
    }

    [[nodiscard]] PileZone *getGraveyardZoneGraphicsItem() const
    {
        return graveyardZoneGraphicsItem;
    }
    [[nodiscard]] PileZone *getRfgZoneGraphicsItem() const
    {
        return rfgZoneGraphicsItem;
    }
    [[nodiscard]] TableZone *getTableZoneGraphicsItem() const
    {
        return tableZoneGraphicsItem;
    }
    [[nodiscard]] StackZone *getStackZoneGraphicsItem() const
    {
        return stackZoneGraphicsItem;
    }
    [[nodiscard]] HandZone *getHandZoneGraphicsItem() const
    {
        return handZoneGraphicsItem;
    }
    [[nodiscard]] CommandZone *getCommandZoneGraphicsItem() const
    {
        return commandZoneGraphicsItem;
    }
    [[nodiscard]] CommandZone *getPartnerZoneGraphicsItem() const
    {
        return partnerZoneGraphicsItem;
    }
    [[nodiscard]] CommandZone *getCompanionZoneGraphicsItem() const
    {
        return companionZoneGraphicsItem;
    }
    [[nodiscard]] CommandZone *getBackgroundZoneGraphicsItem() const
    {
        return backgroundZoneGraphicsItem;
    }

public slots:
    void onPlayerActiveChanged(bool _active);
    void retranslateUi();
    void setCommandZonesVisible(bool visible);
    void setCompanionZoneVisible(bool visible);
    void setBackgroundZoneVisible(bool visible);

signals:
    void sizeChanged();
    void playerCountChanged();

private:
    Player *player;
    PlayerArea *playerArea;
    PlayerTarget *playerTarget;
    PileZone *deckZoneGraphicsItem;
    PileZone *sideboardGraphicsItem;
    PileZone *graveyardZoneGraphicsItem;
    PileZone *rfgZoneGraphicsItem;
    TableZone *tableZoneGraphicsItem;
    StackZone *stackZoneGraphicsItem;
    HandZone *handZoneGraphicsItem;
    CommandZone *commandZoneGraphicsItem;
    CommandZone *partnerZoneGraphicsItem;
    CommandZone *companionZoneGraphicsItem;
    CommandZone *backgroundZoneGraphicsItem;
    QRectF bRect;
    bool mirrored;

private:
    /**
     * @brief Positions command zones and calculates stack zone offset.
     *
     * This helper method centralizes the command zone positioning logic that
     * was previously duplicated across multiple layout branches. It positions
     * the command zone at the given base position, positions the partner zone
     * relative to it, and returns the Y offset where the stack zone should start.
     *
     * @param base The position for the command zone
     * @param commandZonesVisible Whether command zones are enabled
     * @return Y offset from base where stack zone should be positioned
     */
    qreal positionCommandZones(const QPointF &base, bool commandZonesVisible);

private:
    /**
     * @brief Calculates the total height of all visible command zones.
     *
     * Sums the current heights of the primary command zone plus any
     * expanded partner, companion, and background zones.
     *
     * @return Total height in pixels of all visible command zones
     */
    [[nodiscard]] qreal totalCommandZoneHeight() const;

private slots:
    void updateBoundingRect();
    void rearrangeZones();
    void rearrangeCounters();
    /**
     * @brief Handles zone expansion, minimizing the zone above if space is needed.
     *
     * Uses sender() to identify which zone triggered the expansion. When a zone
     * expands and there isn't enough vertical space, this slot calls tryMinimizeAbove()
     * to minimize the zone above instead of the newly-opened zone.
     *
     * @param expanded True if the zone is expanding, false if collapsing
     */
    void onZoneExpanded(bool expanded);
};

#endif // COCKATRICE_PLAYER_GRAPHICS_ITEM_H
