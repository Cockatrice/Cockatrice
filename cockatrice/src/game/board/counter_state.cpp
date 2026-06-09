#include "counter_state.h"

#include <libcockatrice/utility/color.h>

CounterState::CounterState(int id,
                           const QString &name,
                           const QColor &color,
                           int radius,
                           int value,
                           bool active,
                           QObject *parent)
    : QObject(parent), id(id), name(name), color(color), radius(radius), value(value), active(active)
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
