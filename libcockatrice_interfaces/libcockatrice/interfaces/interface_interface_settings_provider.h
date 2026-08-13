#ifndef COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>

/**
 * @struct StoredPlaymat
 * @brief A user-level playmat entry as stored in the settings, independent of
 *        any deck. Mirrors a deck-configured playmat (card + parameters).
 */
struct StoredPlaymat
{
    QString name;                 ///< Card name whose art is used as playmat.
    QString providerId;           ///< Printing provider id (uuid); may be empty.
    double marginPctL = 0.07;     ///< Left margin as fraction of card width (0.0-0.95).
    double marginPctR = 0.07;     ///< Right margin as fraction of card width (0.0-0.95).
    double verticalOffset = 0.33; ///< Vertical position within card (0.0=top, 1.0=bottom).
    double zoom = 1.0;            ///< Scale factor (0.1-4.0).

    bool isEmpty() const
    {
        return name.isEmpty();
    }
};

class IInterfaceSettingsProvider
{
public:
    virtual ~IInterfaceSettingsProvider() = default;

    [[nodiscard]] virtual bool getUseTearOffMenus() const = 0;
    [[nodiscard]] virtual int getCardViewInitialRowsMax() const = 0;
    [[nodiscard]] virtual int getCardViewExpandedRowsMax() const = 0;
    [[nodiscard]] virtual bool getCloseEmptyCardView() const = 0;
    [[nodiscard]] virtual bool getFocusCardViewSearchBar() const = 0;
    [[nodiscard]] virtual bool getKeepGameChatFocus() const = 0;
    [[nodiscard]] virtual bool getNotificationsEnabled() const = 0;
    [[nodiscard]] virtual bool getSpectatorNotificationsEnabled() const = 0;
    [[nodiscard]] virtual bool getBuddyConnectNotificationsEnabled() const = 0;
    [[nodiscard]] virtual bool getDoubleClickToPlay() const = 0;
    [[nodiscard]] virtual bool getClickPlaysAllSelected() const = 0;
    [[nodiscard]] virtual bool getPlayToStack() const = 0;
    [[nodiscard]] virtual bool getDoNotDeleteArrowsInSubPhases() const = 0;
    [[nodiscard]] virtual int getStartingHandSize() const = 0;
    [[nodiscard]] virtual bool getAnnotateTokens() const = 0;
    [[nodiscard]] virtual bool getShowDragSelectionCount() const = 0;
    [[nodiscard]] virtual bool getShowTotalSelectionCount() const = 0;
    [[nodiscard]] virtual int getTallyType() const = 0;
    [[nodiscard]] virtual bool getHorizontalHand() const = 0;
    [[nodiscard]] virtual bool getInvertVerticalCoordinate() const = 0;
    [[nodiscard]] virtual int getMinPlayersForMultiColumnLayout() const = 0;
    [[nodiscard]] virtual int getRewindBufferingMs() const = 0;
    [[nodiscard]] virtual qreal getFastForwardSpeed() const = 0;
    [[nodiscard]] virtual bool getSkipEmptySections() const = 0;
    [[nodiscard]] virtual bool getLeftJustified() const = 0;
    [[nodiscard]] virtual int getZoneViewGroupByIndex() const = 0;
    [[nodiscard]] virtual int getZoneViewSortByIndex() const = 0;
    [[nodiscard]] virtual bool getZoneViewPileView() const = 0;
    [[nodiscard]] virtual bool getShowStatusBar() const = 0;
    [[nodiscard]] virtual bool getShowShortcuts() const = 0;
    [[nodiscard]] virtual bool getShowGameSelectorFilterToolbar() const = 0;
    [[nodiscard]] virtual bool getLifeCounterAnimationsEnabled() const = 0;
    [[nodiscard]] virtual bool getBattlefieldFlashEnabled() const = 0;
    [[nodiscard]] virtual QStringList getUserListExpandedSections() const = 0;
    [[nodiscard]] virtual int getPlaymatVisibility() const = 0;

    /** @brief User-level fallback playmats, used when a deck has none configured. */
    [[nodiscard]] virtual QList<StoredPlaymat> getPlaymatFallbackList() const = 0;

    /** @brief User-level override playmat applied to all decks, or empty to disable. */
    [[nodiscard]] virtual StoredPlaymat getPlaymatOverride() const = 0;

    /** @brief How the fallback list is consulted (0 Fixed, 1 Round-robin, 2 Random). */
    [[nodiscard]] virtual int getPlaymatFallbackMode() const = 0;
};

#endif // COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
