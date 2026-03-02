/**
 * @file player_graphics_item.h
 * @ingroup GameGraphicsPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#define COCKATRICE_PLAYER_GRAPHICS_ITEM_H
#include "../game_scene.h"
#include "player.h"

#include <QGraphicsObject>

class HandZone;
class PileZone;
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

public slots:
    void onPlayerActiveChanged(bool _active);
    void retranslateUi();

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
    QRectF bRect;
    bool mirrored;

private slots:
    void updateBoundingRect();
    void rearrangeZones();
    void rearrangeCounters();
};

#endif // COCKATRICE_PLAYER_GRAPHICS_ITEM_H
