#ifndef COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>
#include <libcockatrice/utility/playmat_params.h>

/**
 * @brief Whether playmats are rendered in-game, and for whom.
 */
enum PlaymatVisibility
{
    PlaymatVisibilityNone = 0,    ///< Don't use playmats.
    PlaymatVisibilityOwnOnly = 1, ///< Show the local player's playmat only.
    PlaymatVisibilityAll = 2      ///< Show playmats for all players.
};

/**
 * @brief How the user-level playmat collection interacts with the deck-configured playmat.
 */
enum PlaymatMode
{
    PlaymatModeOverrideDeck = 0, ///< Always use the collection, ignoring any deck-configured playmat.
    PlaymatModeFallback = 1,     ///< Prefer the deck-configured playmat; fall back to the collection when absent.
    PlaymatModeDeckOnly = 2      ///< Use only the deck-configured playmat, ignoring the collection.
};

/**
 * @brief How the user-level fallback playmat list is consulted when a deck has no playmat configured.
 */
enum PlaymatFallbackMode
{
    PlaymatFallbackModeFixed = 0,      ///< Always use the first entry of the fallback list.
    PlaymatFallbackModeRoundRobin = 1, ///< Cycle through the list, advancing one entry per resolution.
    PlaymatFallbackModeRandom = 2      ///< Pick a random entry per resolution.
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

    /** @brief Who gets playmats rendered: @ref PlaymatVisibility. */
    [[nodiscard]] virtual int getPlaymatVisibility() const = 0;

    /** @brief User-level playmat collection. Used either as a forced playmat
     *         (mode == @ref PlaymatModeOverrideDeck) or as a fallback when a deck has none
     *         (mode == @ref PlaymatModeFallback). */
    [[nodiscard]] virtual QList<PlaymatInfo> getPlaymatFallbackList() const = 0;

    /** @brief How the fallback list is applied: @ref PlaymatMode. */
    [[nodiscard]] virtual int getPlaymatMode() const = 0;

    /** @brief How the fallback list is picked from when mode is @ref PlaymatModeFallback:
     *         @ref PlaymatFallbackMode. */
    [[nodiscard]] virtual int getPlaymatFallbackBehavior() const = 0;
};

#endif // COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
