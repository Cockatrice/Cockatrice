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

    /**
     * @brief Whether this view is a limited look at one end of a library.
     *
     * Only scry/surveil-style views of the top or bottom cards of a deck
     * qualify. Full-zone views, revealed-card views and views of other zones
     * do not, because their first and last cards have no library edge meaning.
     */
    [[nodiscard]] bool isLibraryEdgeView() const;

    /** @brief Whether the first card in the view is the top card of the library. */
    [[nodiscard]] bool showsTopOfLibrary() const
    {
        return isLibraryEdgeView() && !isReversed;
    }

    /** @brief Whether the last card in the view is the bottom card of the library. */
    [[nodiscard]] bool showsBottomOfLibrary() const
    {
        return isLibraryEdgeView() && isReversed;
    }

    CardZoneLogic *getOriginalZone() const
    {
        return origZone;
    }

    /**
     * @brief Builds the zone-dump command for this view without sending it.
     * @return The pending command, or nullptr when there is no zone to dump.
     *
     * Keeps command construction in game/ so graphics views don't reach
     * through the player into PlayerActions to set it up. The response can
     * arrive synchronously (local games), so callers must connect to
     * PendingCommand::finished before calling sendZoneDump().
     */
    PendingCommand *prepareZoneDump();

    /**
     * @brief Sends a command returned by prepareZoneDump().
     * @param pendingCommand The prepared command; nullptr is ignored.
     */
    void sendZoneDump(PendingCommand *pendingCommand);

protected:
    void addCardImpl(CardState *card, int x, int y) override;
};

#endif // COCKATRICE_VIEW_ZONE_LOGIC_H
