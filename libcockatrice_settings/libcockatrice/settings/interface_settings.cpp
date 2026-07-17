#include "interface_settings.h"

InterfaceSettings::InterfaceSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "interface.ini", "interface", QString(), parent)
{
}

bool InterfaceSettings::getUseTearOffMenus() const
{
    return getValue("usetearoffmenus", QString(), QString(), true).toBool();
}

int InterfaceSettings::getCardViewInitialRowsMax() const
{
    return getValue("cardViewInitialRowsMax", QString(), QString(), 14).toInt();
}

int InterfaceSettings::getCardViewExpandedRowsMax() const
{
    return getValue("cardViewExpandedRowsMax", QString(), QString(), 20).toInt();
}

bool InterfaceSettings::getCloseEmptyCardView() const
{
    return getValue("closeEmptyCardView", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getFocusCardViewSearchBar() const
{
    return getValue("focusCardViewSearchBar", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getKeepGameChatFocus() const
{
    return getValue("keepGameChatFocus", QString(), QString(), false).toBool();
}

bool InterfaceSettings::getNotificationsEnabled() const
{
    return getValue("notificationsenabled", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getSpectatorNotificationsEnabled() const
{
    return getValue("specnotificationsenabled", QString(), QString(), false).toBool();
}

bool InterfaceSettings::getBuddyConnectNotificationsEnabled() const
{
    return getValue("buddyconnectnotificationsenabled", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getDoubleClickToPlay() const
{
    return getValue("doubleclicktoplay", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getClickPlaysAllSelected() const
{
    return getValue("clickPlaysAllSelected", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getPlayToStack() const
{
    return getValue("playtostack", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getDoNotDeleteArrowsInSubPhases() const
{
    return getValue("doNotDeleteArrowsInSubPhases", QString(), QString(), true).toBool();
}

int InterfaceSettings::getStartingHandSize() const
{
    return getValue("startinghandsize", QString(), QString(), 7).toInt();
}

bool InterfaceSettings::getAnnotateTokens() const
{
    return getValue("annotatetokens", QString(), QString(), false).toBool();
}

bool InterfaceSettings::getShowDragSelectionCount() const
{
    return getValue("showlassoselectioncount", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getShowTotalSelectionCount() const
{
    return getValue("showpersistentselectioncount", QString(), QString(), true).toBool();
}

int InterfaceSettings::getTallyType() const
{
    return getValue("tallyType", QString(), QString(), 0).toInt();
}

bool InterfaceSettings::getHorizontalHand() const
{
    return getValue("horizontal", "hand", QString(), true).toBool();
}

bool InterfaceSettings::getInvertVerticalCoordinate() const
{
    return getValue("invert_vertical", "table", QString(), false).toBool();
}

int InterfaceSettings::getMinPlayersForMultiColumnLayout() const
{
    return getValue("min_players_multicolumn", QString(), QString(), 4).toInt();
}

bool InterfaceSettings::getOpenDeckInNewTab() const
{
    return getValue("openDeckInNewTab", "editor", QString(), false).toBool();
}

int InterfaceSettings::getRewindBufferingMs() const
{
    return getValue("rewindBufferingMs", "replay", QString(), 200).toInt();
}

bool InterfaceSettings::getStyleUserList() const
{
    return getValue("styleUserList", "appearance", QString(), true).toBool();
}

bool InterfaceSettings::getLeftJustified() const
{
    return getValue("leftjustified", QString(), QString(), false).toBool();
}

int InterfaceSettings::getZoneViewGroupByIndex() const
{
    return getValue("groupby", "zoneview", QString(), 1).toInt();
}

int InterfaceSettings::getZoneViewSortByIndex() const
{
    return getValue("sortby", "zoneview", QString(), 1).toInt();
}

bool InterfaceSettings::getZoneViewPileView() const
{
    return getValue("pileview", "zoneview", QString(), true).toBool();
}

QString InterfaceSettings::getKnownMissingFeatures()
{
    return getValue("knownmissingfeatures", QString(), QString(), "").toString();
}

void InterfaceSettings::setUseTearOffMenus(bool _useTearOffMenus)
{
    setValue(_useTearOffMenus, "usetearoffmenus");
    emit useTearOffMenusChanged(_useTearOffMenus);
}

void InterfaceSettings::setCardViewInitialRowsMax(int _cardViewInitialRowsMax)
{
    setValue(_cardViewInitialRowsMax, "cardViewInitialRowsMax");
}

void InterfaceSettings::setCardViewExpandedRowsMax(int value)
{
    setValue(value, "cardViewExpandedRowsMax");
}

void InterfaceSettings::setCloseEmptyCardView(bool value)
{
    setValue(value, "closeEmptyCardView");
}

void InterfaceSettings::setFocusCardViewSearchBar(bool value)
{
    setValue(value, "focusCardViewSearchBar");
}

void InterfaceSettings::setKeepGameChatFocus(bool value)
{
    setValue(value, "keepGameChatFocus");
    emit keepGameChatFocusChanged(value);
}

void InterfaceSettings::setNotificationsEnabled(bool _notificationsEnabled)
{
    setValue(_notificationsEnabled, "notificationsenabled");
}

void InterfaceSettings::setSpectatorNotificationsEnabled(bool _spectatorNotificationsEnabled)
{
    setValue(_spectatorNotificationsEnabled, "specnotificationsenabled");
}

void InterfaceSettings::setBuddyConnectNotificationsEnabled(bool _buddyConnectNotificationsEnabled)
{
    setValue(_buddyConnectNotificationsEnabled, "buddyconnectnotificationsenabled");
}

void InterfaceSettings::setDoubleClickToPlay(bool _doubleClickToPlay)
{
    setValue(_doubleClickToPlay, "doubleclicktoplay");
}

void InterfaceSettings::setClickPlaysAllSelected(bool _clickPlaysAllSelected)
{
    setValue(_clickPlaysAllSelected, "clickPlaysAllSelected");
}

void InterfaceSettings::setPlayToStack(bool _playToStack)
{
    setValue(_playToStack, "playtostack");
}

void InterfaceSettings::setDoNotDeleteArrowsInSubPhases(bool _doNotDeleteArrowsInSubPhases)
{
    setValue(_doNotDeleteArrowsInSubPhases, "doNotDeleteArrowsInSubPhases");
}

void InterfaceSettings::setStartingHandSize(int _startingHandSize)
{
    setValue(_startingHandSize, "startinghandsize");
}

void InterfaceSettings::setAnnotateTokens(bool _annotateTokens)
{
    setValue(_annotateTokens, "annotatetokens");
}

void InterfaceSettings::setShowDragSelectionCount(bool _showDragSelectionCount)
{
    setValue(_showDragSelectionCount, "showlassoselectioncount");
}

void InterfaceSettings::setShowTotalSelectionCount(bool _showTotalSelectionCount)
{
    setValue(_showTotalSelectionCount, "showpersistentselectioncount");
}

void InterfaceSettings::setTallyType(int value)
{
    if (getTallyType() == value) {
        return;
    }
    setValue(value, "tallyType");
    emit tallyTypeChanged(value);
}

void InterfaceSettings::setHorizontalHand(bool _horizontalHand)
{
    setValue(_horizontalHand, "horizontal", "hand");
    emit horizontalHandChanged();
}

void InterfaceSettings::setInvertVerticalCoordinate(bool _invertVerticalCoordinate)
{
    setValue(_invertVerticalCoordinate, "invert_vertical", "table");
    emit invertVerticalCoordinateChanged();
}

void InterfaceSettings::setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout)
{
    setValue(_minPlayersForMultiColumnLayout, "min_players_multicolumn");
    emit minPlayersForMultiColumnLayoutChanged();
}

void InterfaceSettings::setOpenDeckInNewTab(bool _openDeckInNewTab)
{
    setValue(_openDeckInNewTab, "openDeckInNewTab", "editor");
}

void InterfaceSettings::setRewindBufferingMs(int _rewindBufferingMs)
{
    setValue(_rewindBufferingMs, "rewindBufferingMs", "replay");
}

void InterfaceSettings::setStyleUserList(bool _styleUserList)
{
    setValue(_styleUserList, "styleUserList", "appearance");
    emit styleUserListChanged();
}

void InterfaceSettings::setLeftJustified(bool _leftJustified)
{
    setValue(_leftJustified, "leftjustified");
    emit handJustificationChanged();
}

void InterfaceSettings::setZoneViewGroupByIndex(int _zoneViewGroupByIndex)
{
    setValue(_zoneViewGroupByIndex, "groupby", "zoneview");
}

void InterfaceSettings::setZoneViewSortByIndex(int _zoneViewSortByIndex)
{
    setValue(_zoneViewSortByIndex, "sortby", "zoneview");
}

void InterfaceSettings::setZoneViewPileView(bool _zoneViewPileView)
{
    setValue(_zoneViewPileView, "pileview", "zoneview");
}

void InterfaceSettings::setKnownMissingFeatures(const QString &_knownMissingFeatures)
{
    setValue(_knownMissingFeatures, "knownmissingfeatures");
}
