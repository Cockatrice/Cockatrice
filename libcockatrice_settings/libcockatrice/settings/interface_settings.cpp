#include "interface_settings.h"

namespace
{
const QChar PLAYMAT_FIELD_SEP = QChar(0x1F); ///< Separator between PlaymatInfo fields.

QString encodePlaymatInfo(const PlaymatInfo &res)
{
    return res.card.name + PLAYMAT_FIELD_SEP + res.card.providerId + PLAYMAT_FIELD_SEP +
           QString::number(res.params.marginPctL, 'f', 4) + PLAYMAT_FIELD_SEP +
           QString::number(res.params.marginPctR, 'f', 4) + PLAYMAT_FIELD_SEP +
           QString::number(res.params.verticalOffset, 'f', 4) + PLAYMAT_FIELD_SEP +
           QString::number(res.params.zoom, 'f', 4);
}

PlaymatInfo decodePlaymatInfo(const QString &encoded)
{
    const QStringList fields = encoded.split(PLAYMAT_FIELD_SEP);
    if (fields.size() != 6) {
        return {};
    }
    PlaymatInfo res;
    res.card.name = fields.at(0);
    res.card.providerId = fields.at(1);
    res.params.marginPctL = fields.at(2).toDouble();
    res.params.marginPctR = fields.at(3).toDouble();
    res.params.verticalOffset = fields.at(4).toDouble();
    res.params.zoom = fields.at(5).toDouble();
    return res;
}
} // namespace

InterfaceSettings::InterfaceSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "interface.ini", "interface", QString(), parent)
{
}

bool InterfaceSettings::getUseTearOffMenus() const
{
    return getValue("useTearOffMenus", QString(), QString(), true).toBool();
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
    return getValue("enabled", "interface", "notifications", true).toBool();
}

bool InterfaceSettings::getSpectatorNotificationsEnabled() const
{
    return getValue("spectatorsEnabled", "interface", "notifications", false).toBool();
}

bool InterfaceSettings::getBuddyConnectNotificationsEnabled() const
{
    return getValue("buddyConnectEnabled", "interface", "notifications", true).toBool();
}

bool InterfaceSettings::getDoubleClickToPlay() const
{
    return getValue("doubleClickToPlay", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getClickPlaysAllSelected() const
{
    return getValue("clickPlaysAllSelected", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getPlayToStack() const
{
    return getValue("playToStack", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getDoNotDeleteArrowsInSubPhases() const
{
    return getValue("doNotDeleteArrowsInSubPhases", QString(), QString(), true).toBool();
}

int InterfaceSettings::getStartingHandSize() const
{
    return getValue("startingHandSize", QString(), QString(), 7).toInt();
}

bool InterfaceSettings::getAnnotateTokens() const
{
    return getValue("annotateTokens", QString(), QString(), false).toBool();
}

bool InterfaceSettings::getShowDragSelectionCount() const
{
    return getValue("showLassoSelectionCount", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getShowTotalSelectionCount() const
{
    return getValue("showPersistentSelectionCount", QString(), QString(), true).toBool();
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
    return getValue("invertVertical", "table", QString(), false).toBool();
}

int InterfaceSettings::getMinPlayersForMultiColumnLayout() const
{
    return getValue("minPlayersMulticolumn", QString(), QString(), 4).toInt();
}

int InterfaceSettings::getRewindBufferingMs() const
{
    return getValue("rewindBufferingMs", "replay", QString(), 200).toInt();
}

qreal InterfaceSettings::getFastForwardSpeed() const
{
    return getValue("fastForwardSpeed", "replay", QString(), 10).toReal();
}

bool InterfaceSettings::getSkipEmptySections() const
{
    return getValue("skipEmptySections", "replay", QString(), false).toBool();
}

bool InterfaceSettings::getLeftJustified() const
{
    return getValue("leftJustified", QString(), QString(), false).toBool();
}

int InterfaceSettings::getZoneViewGroupByIndex() const
{
    return getValue("groupBy", "zoneview", QString(), 1).toInt();
}

int InterfaceSettings::getZoneViewSortByIndex() const
{
    return getValue("sortBy", "zoneview", QString(), 1).toInt();
}

bool InterfaceSettings::getZoneViewPileView() const
{
    return getValue("pileView", "zoneview", QString(), true).toBool();
}

bool InterfaceSettings::getShowStatusBar() const
{
    return getValue("showStatusBar", QString(), QString(), false).toBool();
}

bool InterfaceSettings::getShowShortcuts() const
{
    return getValue("showShortcuts", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getShowGameSelectorFilterToolbar() const
{
    return getValue("showGameSelectorFilterToolbar", QString(), QString(), true).toBool();
}

int InterfaceSettings::getPlaymatVisibility() const
{
    return qBound(0, getValue("playmatvisibility", QString(), QString(), 2).toInt(), 2);
}

QList<PlaymatInfo> InterfaceSettings::getPlaymatFallbackList() const
{
    const QStringList entries = getValue("playmatFallbackList", QString(), QString(), QStringList()).toStringList();
    QList<PlaymatInfo> result;
    result.reserve(entries.size());
    for (const QString &entry : entries) {
        const PlaymatInfo res = decodePlaymatInfo(entry);
        if (!res.card.isEmpty()) {
            result.append(res);
        }
    }
    return result;
}

int InterfaceSettings::getPlaymatMode() const
{
    return qBound(0, getValue("playmatMode", QString(), QString(), 1).toInt(), 2);
}

int InterfaceSettings::getPlaymatFallbackBehavior() const
{
    return qBound(0, getValue("playmatFallbackBehavior", QString(), QString(), 0).toInt(), 2);
}

bool InterfaceSettings::getLifeCounterAnimationsEnabled() const
{
    return getValue("lifeCounterAnimationsEnabled", QString(), QString(), true).toBool();
}

bool InterfaceSettings::getBattlefieldFlashEnabled() const
{
    return getValue("battlefieldFlashEnabled", QString(), QString(), true).toBool();
}

QStringList InterfaceSettings::getUserListExpandedSections() const
{
    return getValue("userListExpandedSections", QString(), QString(), QStringList({"buddy", "online", "ignore"}))
        .toStringList();
}

void InterfaceSettings::setUseTearOffMenus(bool _useTearOffMenus)
{
    setValue(_useTearOffMenus, "useTearOffMenus");
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
    setValue(_notificationsEnabled, "enabled", "interface", "notifications");
}

void InterfaceSettings::setSpectatorNotificationsEnabled(bool _spectatorNotificationsEnabled)
{
    setValue(_spectatorNotificationsEnabled, "spectatorsEnabled", "interface", "notifications");
}

void InterfaceSettings::setBuddyConnectNotificationsEnabled(bool _buddyConnectNotificationsEnabled)
{
    setValue(_buddyConnectNotificationsEnabled, "buddyConnectEnabled", "interface", "notifications");
}

void InterfaceSettings::setDoubleClickToPlay(bool _doubleClickToPlay)
{
    setValue(_doubleClickToPlay, "doubleClickToPlay");
}

void InterfaceSettings::setClickPlaysAllSelected(bool _clickPlaysAllSelected)
{
    setValue(_clickPlaysAllSelected, "clickPlaysAllSelected");
}

void InterfaceSettings::setPlayToStack(bool _playToStack)
{
    setValue(_playToStack, "playToStack");
}

void InterfaceSettings::setDoNotDeleteArrowsInSubPhases(bool _doNotDeleteArrowsInSubPhases)
{
    setValue(_doNotDeleteArrowsInSubPhases, "doNotDeleteArrowsInSubPhases");
}

void InterfaceSettings::setStartingHandSize(int _startingHandSize)
{
    setValue(_startingHandSize, "startingHandSize");
}

void InterfaceSettings::setAnnotateTokens(bool _annotateTokens)
{
    setValue(_annotateTokens, "annotateTokens");
}

void InterfaceSettings::setShowDragSelectionCount(bool _showDragSelectionCount)
{
    setValue(_showDragSelectionCount, "showLassoSelectionCount");
}

void InterfaceSettings::setShowTotalSelectionCount(bool _showTotalSelectionCount)
{
    setValue(_showTotalSelectionCount, "showPersistentSelectionCount");
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
    setValue(_invertVerticalCoordinate, "invertVertical", "table");
    emit invertVerticalCoordinateChanged();
}

void InterfaceSettings::setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout)
{
    setValue(_minPlayersForMultiColumnLayout, "minPlayersMulticolumn");
    emit minPlayersForMultiColumnLayoutChanged();
}

void InterfaceSettings::setRewindBufferingMs(int _rewindBufferingMs)
{
    setValue(_rewindBufferingMs, "rewindBufferingMs", "replay");
}

void InterfaceSettings::setFastForwardSpeed(qreal _value)
{
    setValue(_value, "fastForwardSpeed", "replay");
}

void InterfaceSettings::setSkipEmptySections(bool _value)
{
    setValue(_value, "skipEmptySections", "replay");
}

void InterfaceSettings::setLeftJustified(bool _leftJustified)
{
    setValue(_leftJustified, "leftJustified");
    emit handJustificationChanged();
}

void InterfaceSettings::setZoneViewGroupByIndex(int _zoneViewGroupByIndex)
{
    setValue(_zoneViewGroupByIndex, "groupBy", "zoneview");
}

void InterfaceSettings::setZoneViewSortByIndex(int _zoneViewSortByIndex)
{
    setValue(_zoneViewSortByIndex, "sortBy", "zoneview");
}

void InterfaceSettings::setZoneViewPileView(bool _zoneViewPileView)
{
    setValue(_zoneViewPileView, "pileView", "zoneview");
}

void InterfaceSettings::setShowStatusBar(bool _showStatusBar)
{
    setValue(_showStatusBar, "showStatusBar");
    emit showStatusBarChanged(_showStatusBar);
}

void InterfaceSettings::setShowShortcuts(bool _showShortcuts)
{
    setValue(_showShortcuts, "showShortcuts");
}

void InterfaceSettings::setShowGameSelectorFilterToolbar(bool _showGameSelectorFilterToolbar)
{
    setValue(_showGameSelectorFilterToolbar, "showGameSelectorFilterToolbar");
    emit showGameSelectorFilterToolbarChanged(_showGameSelectorFilterToolbar);
}

void InterfaceSettings::setPlaymatVisibility(int _visibility)
{
    if (getPlaymatVisibility() == _visibility) {
        return;
    }
    setValue(_visibility, "playmatvisibility");
    emit playmatVisibilityChanged(_visibility);
}

void InterfaceSettings::setPlaymatFallbackList(const QList<PlaymatInfo> &_fallbackList)
{
    QStringList entries;
    entries.reserve(_fallbackList.size());
    for (const PlaymatInfo &res : _fallbackList) {
        entries.append(encodePlaymatInfo(res));
    }
    setValue(entries, "playmatFallbackList");
    emit playmatSettingsChanged();
}

void InterfaceSettings::setPlaymatMode(int _mode)
{
    const int mode = qBound(0, _mode, 2);
    if (getPlaymatMode() == mode) {
        return;
    }
    setValue(mode, "playmatMode");
    emit playmatSettingsChanged();
}

void InterfaceSettings::setPlaymatFallbackBehavior(int _behavior)
{
    const int behavior = qBound(0, _behavior, 2);
    if (getPlaymatFallbackBehavior() == behavior) {
        return;
    }
    setValue(behavior, "playmatFallbackBehavior");
    emit playmatSettingsChanged();
}

void InterfaceSettings::setLifeCounterAnimationsEnabled(bool _lifeCounterAnimationsEnabled)
{
    setValue(_lifeCounterAnimationsEnabled, "lifeCounterAnimationsEnabled");
    emit lifeCounterAnimationsEnabledChanged(_lifeCounterAnimationsEnabled);
}

void InterfaceSettings::setBattlefieldFlashEnabled(bool _battlefieldFlashEnabled)
{
    setValue(_battlefieldFlashEnabled, "battlefieldFlashEnabled");
    emit battlefieldFlashEnabledChanged(_battlefieldFlashEnabled);
}

void InterfaceSettings::setUserListExpandedSections(const QStringList &sections)
{
    setValue(sections, "userListExpandedSections");
}
