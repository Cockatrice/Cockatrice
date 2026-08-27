/**
 * @file player_target.h
 * @ingroup GameGraphicsPlayers
 */
//! \todo Document this file.

#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "../animated_item.h"
#include "../board/abstract_counter.h"
#include "../board/arrow_target.h"
#include "../board/graphics_item_type.h"

#include <QElapsedTimer>
#include <QPixmap>

class PlayerLogic;

class PlayerCounter : public AbstractCounter, public IAnimatedItem
{
    Q_OBJECT
protected:
    void onValueChanged(int oldValue, int newValue) override;

private:
    static constexpr qreal flashDurationMs = 450.0;

    QElapsedTimer flashClock;
    qreal flashAlpha = 0.0;
    int flashDelta = 0;

public:
    PlayerCounter(CounterState *state, PlayerLogic *player, QGraphicsItem *parent);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    bool animationEvent() override;
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

    explicit PlayerTarget(PlayerLogic *_player = nullptr, QGraphicsItem *parentItem = nullptr);
    ~PlayerTarget() override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    AbstractCounter *addCounter(CounterState *state);
};

#endif
