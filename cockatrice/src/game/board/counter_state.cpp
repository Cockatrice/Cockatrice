#include "counter_state.h"

#include <libcockatrice/utility/color.h>

CounterState::CounterState(int _id,
                           const QString &_name,
                           const QColor &_color,
                           int _radius,
                           int _value,
                           bool _active,
                           QObject *parent)
    : QObject(parent), id(_id), name(_name), color(_color), radius(_radius), value(_value), active(_active)
{
}

CounterState *CounterState::fromProto(const ServerInfo_Counter &counter, QObject *parent)
{
    return new CounterState(counter.id(), QString::fromStdString(counter.name()),
                            convertColorToQColor(counter.counter_color()), counter.radius(), counter.count(),
                            counter.active(), parent);
}

void CounterState::setValue(int newValue)
{
    if (newValue == value) {
        return;
    }
    int old = value;
    value = newValue;
    emit valueChanged(old, newValue);
}

void CounterState::setActive(bool newActive)
{
    if (newActive == active) {
        return;
    }
    active = newActive;
    emit activeChanged(newActive);
}
