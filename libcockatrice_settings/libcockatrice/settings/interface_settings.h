#ifndef INTERFACE_SETTINGS_H
#define INTERFACE_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_interface_settings_provider.h>

class InterfaceSettings : public SettingsManager, public IInterfaceSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getUseTearOffMenus() const override;
    [[nodiscard]] int getCardViewInitialRowsMax() const override;
    [[nodiscard]] int getCardViewExpandedRowsMax() const override;
    [[nodiscard]] bool getCloseEmptyCardView() const override;
    [[nodiscard]] bool getFocusCardViewSearchBar() const override;
    [[nodiscard]] bool getKeepGameChatFocus() const override;
    [[nodiscard]] bool getNotificationsEnabled() const override;
    [[nodiscard]] bool getSpectatorNotificationsEnabled() const override;
    [[nodiscard]] bool getBuddyConnectNotificationsEnabled() const override;
    [[nodiscard]] bool getDoubleClickToPlay() const override;
    [[nodiscard]] bool getClickPlaysAllSelected() const override;
    [[nodiscard]] bool getPlayToStack() const override;
    [[nodiscard]] bool getDoNotDeleteArrowsInSubPhases() const override;
    [[nodiscard]] int getStartingHandSize() const override;
    [[nodiscard]] bool getAnnotateTokens() const override;
    [[nodiscard]] bool getShowDragSelectionCount() const override;
    [[nodiscard]] bool getShowTotalSelectionCount() const override;
    [[nodiscard]] int getTallyType() const override;
    [[nodiscard]] bool getHorizontalHand() const override;
    [[nodiscard]] bool getInvertVerticalCoordinate() const override;
    [[nodiscard]] int getMinPlayersForMultiColumnLayout() const override;
    [[nodiscard]] int getRewindBufferingMs() const override;
    [[nodiscard]] qreal getFastForwardSpeed() const override;
    [[nodiscard]] bool getSkipEmptySections() const override;
    [[nodiscard]] bool getLeftJustified() const override;
    [[nodiscard]] int getZoneViewGroupByIndex() const override;
    [[nodiscard]] int getZoneViewSortByIndex() const override;
    [[nodiscard]] bool getZoneViewPileView() const override;
    [[nodiscard]] bool getShowStatusBar() const override;
    [[nodiscard]] bool getShowShortcuts() const override;
    [[nodiscard]] bool getShowGameSelectorFilterToolbar() const override;
    [[nodiscard]] int getPlaymatVisibility() const override;
    [[nodiscard]] QList<PlaymatInfo> getPlaymatFallbackList() const override;
    [[nodiscard]] int getPlaymatMode() const override;
    [[nodiscard]] int getPlaymatFallbackBehavior() const override;
    [[nodiscard]] bool getLifeCounterAnimationsEnabled() const override;
    [[nodiscard]] bool getBattlefieldFlashEnabled() const override;
    [[nodiscard]] QStringList getUserListExpandedSections() const override;

    void setUseTearOffMenus(bool _useTearOffMenus);
    void setCardViewInitialRowsMax(int _cardViewInitialRowsMax);
    void setCardViewExpandedRowsMax(int value);
    void setCloseEmptyCardView(bool value);
    void setFocusCardViewSearchBar(bool value);
    void setKeepGameChatFocus(bool value);
    void setNotificationsEnabled(bool _notificationsEnabled);
    void setSpectatorNotificationsEnabled(bool _spectatorNotificationsEnabled);
    void setBuddyConnectNotificationsEnabled(bool _buddyConnectNotificationsEnabled);
    void setDoubleClickToPlay(bool _doubleClickToPlay);
    void setClickPlaysAllSelected(bool _clickPlaysAllSelected);
    void setPlayToStack(bool _playToStack);
    void setDoNotDeleteArrowsInSubPhases(bool _doNotDeleteArrowsInSubPhases);
    void setStartingHandSize(int _startingHandSize);
    void setAnnotateTokens(bool _annotateTokens);
    void setShowDragSelectionCount(bool _showDragSelectionCount);
    void setShowTotalSelectionCount(bool _showTotalSelectionCount);
    void setTallyType(int value);
    void setHorizontalHand(bool _horizontalHand);
    void setInvertVerticalCoordinate(bool _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setRewindBufferingMs(int _rewindBufferingMs);
    void setFastForwardSpeed(qreal _value);
    void setSkipEmptySections(bool _value);
    void setLeftJustified(bool _leftJustified);
    void setZoneViewGroupByIndex(int _zoneViewGroupByIndex);
    void setZoneViewSortByIndex(int _zoneViewSortByIndex);
    void setZoneViewPileView(bool _zoneViewPileView);
    void setShowStatusBar(bool _showStatusBar);
    void setShowShortcuts(bool _showShortcuts);
    void setShowGameSelectorFilterToolbar(bool _showGameSelectorFilterToolbar);
    void setPlaymatVisibility(int _visibility);
    void setPlaymatFallbackList(const QList<PlaymatInfo> &_fallbackList);
    void setPlaymatMode(int _mode);
    void setPlaymatFallbackBehavior(int _behavior);
    void setLifeCounterAnimationsEnabled(bool _lifeCounterAnimationsEnabled);
    void setBattlefieldFlashEnabled(bool _battlefieldFlashEnabled);
    void setUserListExpandedSections(const QStringList &sections);

signals:
    void useTearOffMenusChanged(bool state);
    void keepGameChatFocusChanged(bool value);
    void horizontalHandChanged();
    void invertVerticalCoordinateChanged();
    void minPlayersForMultiColumnLayoutChanged();
    void handJustificationChanged();
    void tallyTypeChanged(int type);
    void showStatusBarChanged(bool state);
    void showGameSelectorFilterToolbarChanged(bool state);
    void playmatVisibilityChanged(int visibility);
    void playmatSettingsChanged();
    void lifeCounterAnimationsEnabledChanged(bool state);
    void battlefieldFlashEnabledChanged(bool state);

public:
    explicit InterfaceSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    InterfaceSettings(const InterfaceSettings & /*other*/);
};

#endif // INTERFACE_SETTINGS_H
