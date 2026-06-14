#include "card_state.h"

#include "../player/player_logic.h"
#include "../zones/card_zone_logic.h"

CardState::CardState(PlayerLogic *_owner, const CardRef &cardRef, int _id, CardZoneLogic *_zone)
    : AbstractCardState(_owner, cardRef, _id), zone(_zone)
{
    owner->addCard(this);
}

void CardState::prepareDelete()
{
    while (!attachedCards.isEmpty()) {
        attachedCards.first()->setZone(nullptr); // so that it won't try to call reorganizeCards()
        attachedCards.first()->setAttachedTo(nullptr);
    }

    if (getAttachedTo() != nullptr) {
        getAttachedTo()->removeAttachedCard(this);
        setAttachedTo(nullptr);
    }
}

void CardState::deleteLater()
{
    prepareDelete();
    QObject::deleteLater();
}

void CardState::processCardInfo(const ServerInfo_Card &_info)
{
    clearCounters();
    const int counterListSize = _info.counter_list_size();
    for (int i = 0; i < counterListSize; ++i) {
        const ServerInfo_CardCounter &counterInfo = _info.counter_list(i);
        insertCounter(counterInfo.id(), counterInfo.value());
    }

    setId(_info.id());
    setCardRef({QString::fromStdString(_info.name()), QString::fromStdString(_info.provider_id())});
    setAttacking(_info.attacking());
    setFaceDown(_info.face_down());
    setPT(QString::fromStdString(_info.pt()));
    setAnnotation(QString::fromStdString(_info.annotation()));
    setColor(QString::fromStdString(_info.color()));
    setTapped(_info.tapped());
    setDestroyOnZoneChange(_info.destroy_on_zone_change());
    setDoesntUntap(_info.doesnt_untap());
}

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

void CardState::clearAttachedCards()
{
    attachedCards.clear();
}

void CardState::setZone(CardZoneLogic *_zone)
{
    if (zone == _zone) {
        return;
    }

    zone = _zone;
    emit zoneChanged(this, zone);
    emit stateChanged();
}

void CardState::setAttacking(bool _attacking)
{
    if (attacking == _attacking) {
        return;
    }
    attacking = _attacking;
    emit attackingChanged(_attacking);
    emit stateChanged();
}

void CardState::insertCounter(int id, int value)
{
    counters.insert(id, value);

    emit countersChanged(counters);
    emit stateChanged();
}

void CardState::setCounter(int id, int value)
{
    if (value) {
        counters[id] = value;
    } else {
        counters.remove(id);
    }

    emit countersChanged(counters);
    emit stateChanged();
}

void CardState::clearCounters()
{
    counters.clear();
    emit countersChanged(counters);
    emit stateChanged();
}

void CardState::setAnnotation(const QString &_annotation)
{
    if (annotation == _annotation) {
        return;
    }
    annotation = _annotation;
    emit annotationChanged(annotation);
    emit stateChanged();
}

void CardState::setPT(const QString &_pt)
{
    if (pt == _pt) {
        return;
    }
    pt = _pt;
    emit ptChanged(pt);
    emit stateChanged();
}

void CardState::setDoesntUntap(bool _doesntUntap)
{
    if (doesntUntap == _doesntUntap) {
        return;
    }
    doesntUntap = _doesntUntap;
    emit doesntUntapChanged(_doesntUntap);
    emit stateChanged();
}

void CardState::setDestroyOnZoneChange(bool _destroyOnZoneChange)
{
    if (destroyOnZoneChange == _destroyOnZoneChange) {
        return;
    }

    destroyOnZoneChange = _destroyOnZoneChange;
    emit destroyOnZoneChangeChanged(_destroyOnZoneChange);
    emit stateChanged();
}

void CardState::setAttachedTo(CardState *_attachedTo)
{
    if (attachedTo == _attachedTo) {
        return;
    }
    attachedTo = _attachedTo;
    emit attachedToChanged(_attachedTo);
    emit stateChanged();
}

/*
void CardItem::setAttachedTo(CardItem *_attachedTo)
{
    if (state->getAttachedTo() != nullptr) {
        state->getAttachedTo()->removeAttachedCard(this);
    }

    gridPoint.setX(-1);
    state->setAttachedTo(_attachedTo);
    if (state->getAttachedTo() != nullptr) {
        // If the zone is being torn down, it might already be null by the time a card tries to un-attach all its
        // attached cards
        if (state->getAttachedTo()->getState()->getZone() == nullptr) {
            deleteLater();
        } else {
            emit state->getAttachedTo()->getState()->getZone()->cardAdded(this);
            state->getAttachedTo()->addAttachedCard(this);
            if (state->getZone() != state->getAttachedTo()->getState()->getZone()) {
                state->getAttachedTo()->getState()->getZone()->reorganizeCards();
            }
        }
    } else {
        // If the zone is being torn down, it might already be null by the time a card tries to un-attach all its
        // attached cards
        if (state->getZone() == nullptr) {
            deleteLater();
        } else {
            emit state->getZone()->cardAdded(this);
        }
    }

    if (state->getZone() != nullptr) {
        state->getZone()->reorganizeCards();
    }
}*/