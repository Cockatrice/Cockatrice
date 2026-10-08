/**
 * @file view_zone_logic.h
 * @ingroup GameLogicZones
 */
//! \todo Document this file.

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

    ZoneViewZoneLogic(PlayerLogic *_player,
                      CardZoneLogic *_origZone,
                      int _numberCards,
                      bool _revealZone,
                      bool _writeableRevealZone,
                      bool _isReversed,
                      QObject *parent = nullptr);

    bool prepareAddCard(int x);
    void removeCard(int position, bool toNewZone);
    void updateCardIds(CardAction action);

    /**
     * @brief Asks the graphics view showing this zone to close itself.
     *
     * Logic code has no handle on the view, so it goes through the logic object the view was built from.
     */
    void requestClose();

    /** @brief Removes all cards from the view, without touching anything else. Used in replay rewind */
    void clearCards();

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
    void addCardImpl(CardState *card, int x, int y) override;
};

#endif // COCKATRICE_VIEW_ZONE_LOGIC_H
