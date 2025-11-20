/**
 * @file player_target.h
 * @ingroup GameGraphicsPlayers
 * @brief TODO: Document this.
 */

#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "../../game_graphics/board/graphics_item_type.h"
#include "../board/abstract_counter.h"
#include "../board/arrow_target.h"

#include <QFont>
#include <QPixmap>

class Player;

class PlayerCounter : public AbstractCounter
{
    Q_OBJECT
public:
    PlayerCounter(Player *_player, int _id, const QString &_name, int _value, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

class PlayerTarget : public ArrowTarget
{
    Q_OBJECT
private:
    QPixmap fullPixmap;
    PlayerCounter *playerCounter;
public slots:
    void counterDeleted();

public:
    enum
    {
        Type = typePlayerTarget
    };
    int type() const override
    {
        return Type;
    }

    explicit PlayerTarget(Player *_player = nullptr, QGraphicsItem *parentItem = nullptr);
    ~PlayerTarget() override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    AbstractCounter *addCounter(int _counterId, const QString &_name, int _value);
};

#endif
