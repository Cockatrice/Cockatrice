/**
 * @file player_area.h
 * @ingroup GameGraphicsPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_AREA_H
#define COCKATRICE_PLAYER_AREA_H

#include "../board/graphics_item_type.h"
#include "QGraphicsItem"

/**
 * The entire graphical area belonging to a single player.
 */
class PlayerArea : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QRectF bRect;
    int playerZoneId;
private slots:
    void updateBg();

public:
    enum
    {
        Type = typeOther
    };
    [[nodiscard]] int type() const override
    {
        return Type;
    }

    explicit PlayerArea(QGraphicsItem *parent = nullptr);
    [[nodiscard]] QRectF boundingRect() const override
    {
        return bRect;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setSize(qreal width, qreal height);

    void setPlayerZoneId(int _playerZoneId);
    [[nodiscard]] int getPlayerZoneId() const
    {
        return playerZoneId;
    }
};

#endif // COCKATRICE_PLAYER_AREA_H
