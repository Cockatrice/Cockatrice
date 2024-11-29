
#include "mocks.h"

CardDatabaseSettings::CardDatabaseSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardDatabase.ini", parent)
{
}
void CardDatabaseSettings::setSortKey(QString /* shortName */, unsigned int /* sortKey */)
{
}
void CardDatabaseSettings::setEnabled(QString /* shortName */, bool /* enabled */)
{
}
void CardDatabaseSettings::setIsKnown(QString /* shortName */, bool /* isknown */)
{
}
unsigned int CardDatabaseSettings::getSortKey(QString /* shortName */)
{
    return 0;
};
bool CardDatabaseSettings::isEnabled(QString /* shortName */)
{
    return true;
};
bool CardDatabaseSettings::isKnown(QString /* shortName */)
{
    return true;
};

QString SettingsCache::getDataPath()
{
    return "";
}
QString SettingsCache::getSettingsPath()
{
    return "";
}
void SettingsCache::translateLegacySettings()
{
}
QString SettingsCache::getSafeConfigPath(QString /* configEntry */, QString defaultPath) const
{
    return defaultPath;
}
QString SettingsCache::getSafeConfigFilePath(QString /* configEntry */, QString defaultPath) const
{
    return defaultPath;
}
SettingsCache::SettingsCache()
    : settings{new QSettings("global.ini", QSettings::IniFormat, this)}, shortcutsSettings{nullptr},
      cardDatabaseSettings{new CardDatabaseSettings("", this)}, serversSettings{nullptr}, messageSettings{nullptr},
      gameFiltersSettings{nullptr}, layoutsSettings{nullptr}, downloadSettings{nullptr}
{
}
void SettingsCache::setUseTearOffMenus(bool /* _useTearOffMenus */)
{
}
void SettingsCache::setKnownMissingFeatures(const QString & /* _knownMissingFeatures */)
{
}
void SettingsCache::setCardInfoViewMode(const int /* _viewMode */)
{
}
void SettingsCache::setHighlightWords(const QString & /* _highlightWords */)
{
}
void SettingsCache::setMasterVolume(int /* _masterVolume */)
{
}
void SettingsCache::setLeftJustified(const QT_STATE_CHANGED_T /* _leftJustified */)
{
}
void SettingsCache::setCardScaling(const QT_STATE_CHANGED_T /* _scaleCards */)
{
}
void SettingsCache::setStackCardOverlapPercent(const int /* _verticalCardOverlapPercent */)
{
}
void SettingsCache::setShowMessagePopups(const QT_STATE_CHANGED_T /* _showMessagePopups */)
{
}
void SettingsCache::setShowMentionPopups(const QT_STATE_CHANGED_T /* _showMentionPopus */)
{
}
void SettingsCache::setRoomHistory(const QT_STATE_CHANGED_T /* _roomHistory */)
{
}
void SettingsCache::setLang(const QString & /* _lang */)
{
}
void SettingsCache::setShowTipsOnStartup(bool /* _showTipsOnStartup */)
{
}
void SettingsCache::setSeenTips(const QList<int> & /* _seenTips */)
{
}
void SettingsCache::setDeckPath(const QString & /* _deckPath */)
{
}
void SettingsCache::setReplaysPath(const QString & /* _replaysPath */)
{
}
void SettingsCache::setThemesPath(const QString & /* _themesPath */)
{
}
void SettingsCache::setPicsPath(const QString & /* _picsPath */)
{
}
void SettingsCache::setCardDatabasePath(const QString & /* _cardDatabasePath */)
{
}
void SettingsCache::setCustomCardDatabasePath(const QString & /* _customCardDatabasePath */)
{
}
void SettingsCache::setSpoilerDatabasePath(const QString & /* _spoilerDatabasePath */)
{
}
void SettingsCache::setTokenDatabasePath(const QString & /* _tokenDatabasePath */)
{
}
void SettingsCache::setThemeName(const QString & /* _themeName */)
{
}
void SettingsCache::setPicDownload(QT_STATE_CHANGED_T /* _picDownload */)
{
}
void SettingsCache::setNotificationsEnabled(QT_STATE_CHANGED_T /* _notificationsEnabled */)
{
}
void SettingsCache::setSpectatorNotificationsEnabled(QT_STATE_CHANGED_T /* _spectatorNotificationsEnabled */)
{
}
void SettingsCache::setBuddyConnectNotificationsEnabled(QT_STATE_CHANGED_T /* _buddyConnectNotificationsEnabled */)
{
}
void SettingsCache::setDoubleClickToPlay(QT_STATE_CHANGED_T /* _doubleClickToPlay */)
{
}
void SettingsCache::setPlayToStack(QT_STATE_CHANGED_T /* _playToStack */)
{
}
void SettingsCache::setStartingHandSize(int /* _startingHandSize */)
{
}
void SettingsCache::setAnnotateTokens(QT_STATE_CHANGED_T /* _annotateTokens */)
{
}
void SettingsCache::setTabGameSplitterSizes(const QByteArray & /* _tabGameSplitterSizes */)
{
}
void SettingsCache::setDisplayCardNames(QT_STATE_CHANGED_T /* _displayCardNames */)
{
}
void SettingsCache::setHorizontalHand(QT_STATE_CHANGED_T /* _horizontalHand */)
{
}
void SettingsCache::setInvertVerticalCoordinate(QT_STATE_CHANGED_T /* _invertVerticalCoordinate */)
{
}
void SettingsCache::setMinPlayersForMultiColumnLayout(int /* _minPlayersForMultiColumnLayout */)
{
}
void SettingsCache::setTapAnimation(QT_STATE_CHANGED_T /* _tapAnimation */)
{
}
void SettingsCache::setOpenDeckInNewTab(QT_STATE_CHANGED_T /* _openDeckInNewTab */)
{
}
void SettingsCache::setChatMention(QT_STATE_CHANGED_T /* _chatMention */)
{
}
void SettingsCache::setChatMentionCompleter(const QT_STATE_CHANGED_T /* _enableMentionCompleter */)
{
}
void SettingsCache::setChatMentionForeground(QT_STATE_CHANGED_T /* _chatMentionForeground */)
{
}
void SettingsCache::setChatHighlightForeground(QT_STATE_CHANGED_T /* _chatHighlightForeground */)
{
}
void SettingsCache::setChatMentionColor(const QString & /* _chatMentionColor */)
{
}
void SettingsCache::setChatHighlightColor(const QString & /* _chatHighlightColor */)
{
}
void SettingsCache::setZoneViewGroupByIndex(int /* _zoneViewGroupByIndex */)
{
}
void SettingsCache::setZoneViewSortByIndex(int /* _zoneViewSortByIndex */)
{
}
void SettingsCache::setZoneViewPileView(QT_STATE_CHANGED_T /* _zoneViewPileView */)
{
}
void SettingsCache::setSoundEnabled(QT_STATE_CHANGED_T /* _soundEnabled */)
{
}
void SettingsCache::setSoundThemeName(const QString & /* _soundThemeName */)
{
}
void SettingsCache::setIgnoreUnregisteredUsers(QT_STATE_CHANGED_T /* _ignoreUnregisteredUsers */)
{
}
void SettingsCache::setIgnoreUnregisteredUserMessages(QT_STATE_CHANGED_T /* _ignoreUnregisteredUserMessages */)
{
}
void SettingsCache::setMainWindowGeometry(const QByteArray & /* _mainWindowGeometry */)
{
}
void SettingsCache::setTokenDialogGeometry(const QByteArray & /* _tokenDialogGeometry */)
{
}
void SettingsCache::setSetsDialogGeometry(const QByteArray & /* _setsDialogGeometry */)
{
}
void SettingsCache::setPixmapCacheSize(const int /* _pixmapCacheSize */)
{
}
void SettingsCache::setNetworkCacheSizeInMB(const int /* _networkCacheSize */)
{
}
void SettingsCache::setNetworkRedirectCacheTtl(const int /* _redirectCacheTtl */)
{
}
void SettingsCache::setClientID(const QString & /* _clientID */)
{
}
void SettingsCache::setClientVersion(const QString & /* _clientVersion */)
{
}
QStringList SettingsCache::getCountries() const
{
    static QStringList countries = QStringList() << "us";
    return countries;
}
void SettingsCache::setGameDescription(const QString /* _gameDescription */)
{
}
void SettingsCache::setMaxPlayers(const int /* _maxPlayers */)
{
}
void SettingsCache::setGameTypes(const QString /* _gameTypes */)
{
}
void SettingsCache::setOnlyBuddies(const bool /* _onlyBuddies */)
{
}
void SettingsCache::setOnlyRegistered(const bool /* _onlyRegistered */)
{
}
void SettingsCache::setSpectatorsAllowed(const bool /* _spectatorsAllowed */)
{
}
void SettingsCache::setSpectatorsNeedPassword(const bool /* _spectatorsNeedPassword */)
{
}
void SettingsCache::setSpectatorsCanTalk(const bool /* _spectatorsCanTalk */)
{
}
void SettingsCache::setSpectatorsCanSeeEverything(const bool /* _spectatorsCanSeeEverything */)
{
}
void SettingsCache::setCreateGameAsSpectator(const bool /* _createGameAsSpectator */)
{
}
void SettingsCache::setRememberGameSettings(const bool /* _rememberGameSettings */)
{
}
void SettingsCache::setNotifyAboutUpdate(QT_STATE_CHANGED_T /* _notifyaboutupdate */)
{
}
void SettingsCache::setNotifyAboutNewVersion(QT_STATE_CHANGED_T /* _notifyaboutnewversion */)
{
}
void SettingsCache::setDownloadSpoilerStatus(bool /* _spoilerStatus */)
{
}
void SettingsCache::setUpdateReleaseChannel(int /* _updateReleaseChannel */)
{
}
void SettingsCache::setMaxFontSize(int /* _max */)
{
}

void PictureLoader::clearPixmapCache(CardInfoPtr /* card */)
{
}

SettingsCache *settingsCache;

SettingsCache &SettingsCache::instance()
{
    return *settingsCache;
}
