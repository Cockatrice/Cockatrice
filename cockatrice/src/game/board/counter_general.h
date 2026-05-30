/**
 * @file counter_general.h
 * @ingroup GameGraphicsPlayers
 */
//! \todo Document this file.

#ifndef COUNTER_GENERAL_H
#define COUNTER_GENERAL_H

#include "abstract_counter.h"

class GeneralCounter : public AbstractCounter
{
    Q_OBJECT

public:
    GeneralCounter(CounterState *state,
                   PlayerLogic *player,
                   bool useNameForShortcut = false,
                   QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
