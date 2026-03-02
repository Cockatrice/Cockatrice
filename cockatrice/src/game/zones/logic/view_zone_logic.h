/**
 * @file view_zone_logic.h
 * @ingroup GameLogicZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_VIEW_ZONE_LOGIC_H
#define COCKATRICE_VIEW_ZONE_LOGIC_H
#include "card_zone_logic.h"

class ZoneViewZoneLogic : public CardZoneLogic
{
    Q_OBJECT
signals:
    void addToViews();
    void removeFromViews();
    void closeView();

private:
    CardZoneLogic *origZone;
    int numberCards;
    bool revealZone, writeableRevealZone;
    bool isReversed;

public:
    enum CardAction
    {
        INITIALIZE,
        ADD_CARD,
        REMOVE_CARD
    };

    ZoneViewZoneLogic(Player *_player,
                      CardZoneLogic *_origZone,
                      int _numberCards,
                      bool _revealZone,
                      bool _writeableRevealZone,
                      bool _isReversed,
                      QObject *parent = nullptr);

    bool prepareAddCard(int x);
    void removeCard(int position, bool toNewZone);
    void updateCardIds(CardAction action);
    int getNumberCards() const
    {
        return numberCards;
    }
    bool getRevealZone() const
    {
        return revealZone;
    }
    bool getWriteableRevealZone() const
    {
        return writeableRevealZone;
    }
    void setWriteableRevealZone(bool _writeableRevealZone);
    bool getIsReversed() const
    {
        return isReversed;
    }

    CardZoneLogic *getOriginalZone() const
    {
        return origZone;
    }

protected:
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif // COCKATRICE_VIEW_ZONE_LOGIC_H
