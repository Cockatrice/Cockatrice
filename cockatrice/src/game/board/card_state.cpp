#include "card_state.h"

#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>

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

void CardState::setCardRef(const CardRef &_cardRef)
{
    if (cardRef == _cardRef) {
        return;
    }

    const CardRef oldCardRef = cardRef;
    if (exactCard) {
        disconnect(exactCard.getCardPtr().data(), nullptr, this, nullptr);
    }
    cardRef = _cardRef;

    emit cardRefChanged(oldCardRef, cardRef);
    refreshCardInfo();
}

void CardState::refreshCardInfo()
{
    exactCard = CardDatabaseManager::query()->getCard(cardRef);

    if (!exactCard && !cardRef.name.isEmpty()) {
        CardInfo::UiAttributes attributes = {.tableRow = -1};
        auto info = CardInfo::newInstance(cardRef.name, "", true, {}, {}, {}, {}, attributes);
        exactCard = ExactCard(info);
    }
    if (exactCard) {
        connect(exactCard.getCardPtr().data(), &CardInfo::pixmapUpdated, this, &CardState::cardPixmapUpdated);
    }

    emit cardInfoChanged();
}

void CardState::setTapped(bool _tapped, bool canAnimate)
{
    if (tapped == _tapped) {
        return;
    }

    tapped = _tapped;
    emit tappedChanged(tapped, canAnimate);
    emit stateChanged();
}

void CardState::setFaceDown(bool _facedown)
{
    facedown = _facedown;
    emit facedownChanged(facedown);
    emit stateChanged();
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

void CardState::setAttachedTo(CardItem *_attachedTo)
{
    if (attachedTo == _attachedTo) {
        return;
    }
    attachedTo = _attachedTo;
    emit attachedToChanged(_attachedTo);
    emit stateChanged();
}
