/**
 * @file view_zone_logic.h
 * @ingroup GameLogicZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_VIEW_ZONE_LOGIC_H
#define COCKATRICE_VIEW_ZONE_LOGIC_H
#include "card_zone_logic.h"

#include <qtmetamacros.h>

class PlayerLogic;
class QObject;
class PendingCommand;
class IInterfaceSettingsProvider;

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
    // Borrowed settings provider (owned by the application). Null outside the client, in which case views stay open.
    IInterfaceSettingsProvider *interfaceSettings;

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
                      IInterfaceSettingsProvider *interfaceSettings = nullptr,
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

    [[nodiscard]] bool getCloseEmptyCardView() const;

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

    /**
     * @brief Prepares and sends the zone-dump command for this view.
     * @return The pending command, or nullptr when there is no zone to dump.
     *
     * Keeps command construction in game/ so graphics views don't reach
     * through the player into PlayerActions to set it up. The command is sent
     * asynchronously, so callers connect to finished() after calling this.
     */
    PendingCommand *requestZoneDump();

protected:
    void addCardImpl(CardState *card, int x, int y) override;
};

#endif // COCKATRICE_VIEW_ZONE_LOGIC_H
