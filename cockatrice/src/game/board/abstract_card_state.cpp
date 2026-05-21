#include "abstract_card_state.h"

#include "libcockatrice/card/database/card_database_manager.h"

AbstractCardState::AbstractCardState(PlayerLogic *_owner, const CardRef &_cardRef, int _id)
    : owner(_owner), cardRef(_cardRef), id(_id), tapped(false), facedown(false)
{
    refreshCardInfo();
}

/**
 * Convenience method to get the CardInfo of the exactCard
 * @return A const reference to the CardInfo, or an empty CardInfo if card was null
 */
const CardInfo &AbstractCardState::getCardInfo() const
{
    return exactCard.getInfo();
}

void AbstractCardState::refreshCardInfo()
{
    exactCard = CardDatabaseManager::query()->getCard(cardRef);

    if (!exactCard && !cardRef.name.isEmpty()) {
        CardInfo::UiAttributes attributes = {.tableRow = -1};
        auto info = CardInfo::newInstance(cardRef.name, "", true, {}, {}, {}, {}, attributes);
        exactCard = ExactCard(info);
    }

    emit cardInfoRefreshed(exactCard);

    emit colorChanged();
}

void AbstractCardState::setCardRef(const CardRef &_cardRef)
{
    if (cardRef == _cardRef) {
        return;
    }

    // TODO emit deleteCardInfoPopup(cardRef.name);
    if (exactCard) {
        disconnect(exactCard.getCardPtr().data(), nullptr, this, nullptr);
    }
    cardRef = _cardRef;

    refreshCardInfo();
}

void AbstractCardState::setTapped(bool _tapped, bool canAnimate)
{
    if (tapped == _tapped) {
        return;
    }

    tapped = _tapped;
    Q_UNUSED(canAnimate);
    // TODO
    /*if (SettingsCache::instance().getTapAnimation() && canAnimate) {
        static_cast<GameScene *>(scene())->registerAnimationItem(this);
    } else {
        tapAngle = tapped ? 90 : 0;
        setTransform(QTransform()
                         .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                         .rotate(tapAngle)
                         .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));
        update();
    }*/
}

void AbstractCardState::setFaceDown(bool _facedown)
{
    facedown = _facedown;
    emit faceDownChanged(facedown);
}

void AbstractCardState::setColor(const QString &_color)
{
    color = _color;
    emit colorChanged();
}