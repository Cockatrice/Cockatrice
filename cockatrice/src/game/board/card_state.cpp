#include "card_state.h"

void CardState::resetState(bool keepAnnotations)
{
    attacking = false;
    counters.clear();
    pt.clear();
    if (!keepAnnotations) {
        annotation.clear();
    }
    attachedTo = nullptr;
}

void CardState::setZone(CardZoneLogic *_zone)
{
    if (zone == _zone) {
        return;
    }

    zone = _zone;
    emit zoneChanged(zone);
    emit stateChanged();
}

void CardState::setAttacking(bool v)
{
    if (attacking == v) {
        return;
    }
    attacking = v;
    emit attackingChanged(v);
    emit stateChanged();
}

void CardState::insertCounter(int id, int value)
{
    counters.insert(id, value);

    emit countersChanged();
    emit stateChanged();
}

void CardState::setCounter(int id, int value)
{
    if (value) {
        counters[id] = value;
    } else {
        counters.remove(id);
    }

    emit countersChanged();
    emit stateChanged();
}

void CardState::clearCounters()
{
    counters.clear();
    emit countersChanged();
    emit stateChanged();
}

void CardState::setAnnotation(const QString &a)
{
    if (annotation == a) {
        return;
    }
    annotation = a;
    emit annotationChanged();
    emit stateChanged();
}

void CardState::setPT(const QString &v)
{
    if (pt == v) {
        return;
    }
    pt = v;
    emit ptChanged();
    emit stateChanged();
}

void CardState::setDoesntUntap(bool v)
{
    if (doesntUntap == v) {
        return;
    }
    doesntUntap = v;
    emit doesntUntapChanged(v);
    emit stateChanged();
}

void CardState::setDestroyOnZoneChange(bool v)
{
    if (destroyOnZoneChange == v) {
        return;
    }

    destroyOnZoneChange = v;
    emit destroyOnZoneChangeChanged(v);
    emit stateChanged();
}

void CardState::setAttachedTo(CardItem *c)
{
    if (attachedTo == c) {
        return;
    }
    attachedTo = c;
    emit attachedToChanged(c);
    emit stateChanged();
}