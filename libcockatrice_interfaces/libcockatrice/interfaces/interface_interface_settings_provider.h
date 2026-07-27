#ifndef COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H

#include <QString>

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
    [[nodiscard]] virtual bool getOpenDeckInNewTab() const = 0;
    [[nodiscard]] virtual int getRewindBufferingMs() const = 0;
    [[nodiscard]] virtual qreal getFastForwardSpeed() const = 0;
    [[nodiscard]] virtual bool getStyleUserList() const = 0;
    [[nodiscard]] virtual bool getLeftJustified() const = 0;
    [[nodiscard]] virtual int getZoneViewGroupByIndex() const = 0;
    [[nodiscard]] virtual int getZoneViewSortByIndex() const = 0;
    [[nodiscard]] virtual bool getZoneViewPileView() const = 0;
    [[nodiscard]] virtual QString getKnownMissingFeatures() = 0;
};

#endif // COCKATRICE_INTERFACE_INTERFACE_SETTINGS_PROVIDER_H
