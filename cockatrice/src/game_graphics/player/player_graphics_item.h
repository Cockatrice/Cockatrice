/**
 * @file player_graphics_item.h
 * @ingroup GameGraphicsPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#define COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#include "../../game/player/player_logic.h"
#include "../board/abstract_counter.h"
#include "../game_scene.h"

#include <QGraphicsObject>

class CommandZone;
class HandZone;
class PileZone;
class PlayerDialogs;
class PlayerMenu;
class PlayerTarget;
class StackZone;
class TableZone;
class ZoneViewZone;

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

    explicit PlayerGraphicsItem(PlayerLogic *player);
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

    PlayerLogic *getLogic() const
    {
        return player;
    }

    [[nodiscard]] PlayerMenu *getPlayerMenu() const
    {
        return playerMenu;
    }

    PlayerArea *getPlayerArea() const
    {
        return playerArea;
    }

    PlayerTarget *getPlayerTarget() const
    {
        return playerTarget;
    }

    CardZone *getZoneGraphicsItem(const QString &name) const
    {
        return zoneGraphicsItems.value(name, nullptr);
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
    /** @brief Returns the command zone graphics item. */
    [[nodiscard]] CommandZone *getCommandZoneGraphicsItem() const
    {
        return commandZoneGraphicsItem;
    }
    /** @brief Returns the counter widget for the given counter ID, or nullptr if not found. */
    [[nodiscard]] AbstractCounter *getCounterWidget(int counterId) const
    {
        return counterWidgets.value(counterId, nullptr);
    }
    /** @brief Returns the tax counter if it exists and is active, or nullptr otherwise. */
    [[nodiscard]] AbstractCounter *getTaxCounterIfActive(int counterId) const;

public slots:
    void onPlayerActiveChanged(bool _active);
    void onCustomZoneAdded(QString customZoneName);
    void onCounterAdded(CounterState *state);
    void onCounterRemoved(int counterId);
    void rearrangeCounters();
    void retranslateUi();
    /** @brief Shows or hides the command zone and rearranges dependent zones. */
    void setCommandZoneVisible(bool visible);

signals:
    void sizeChanged();
    void playerCountChanged();
    void mirroredChanged(bool isMirrored);
    void cardInfoRequested(const CardRef &cardRef);

private:
    PlayerLogic *player;
    PlayerMenu *playerMenu;
    PlayerDialogs *playerDialogs;
    PlayerArea *playerArea;
    PlayerTarget *playerTarget;
    QMap<int, AbstractCounter *> counterWidgets;
    QMap<QString, CardZone *> zoneGraphicsItems;
    PileZone *deckZoneGraphicsItem;
    PileZone *sideboardGraphicsItem;
    PileZone *graveyardZoneGraphicsItem;
    PileZone *rfgZoneGraphicsItem;
    TableZone *tableZoneGraphicsItem;
    StackZone *stackZoneGraphicsItem;
    HandZone *handZoneGraphicsItem;
    CommandZone *commandZoneGraphicsItem = nullptr;
    QRectF bRect;
    bool mirrored;
    bool handVisible = false;

    /** @brief Returns the command zone's display height, or 0 if hidden. */
    [[nodiscard]] qreal totalCommandZoneHeight() const;
    /** @brief Positions the command and stack zones vertically starting from base, updating base.y. */
    void positionCommandAndStackZones(const QPointF &base);
private slots:
    void updateBoundingRect();
    void rearrangeZones();
};

#endif // COCKATRICE_PLAYER_GRAPHICS_ITEM_H
