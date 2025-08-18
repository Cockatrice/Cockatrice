
#include "mocks.h"

CardDatabaseSettings::CardDatabaseSettings(const QString &settingPath, QObject *parent)
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
      gameFiltersSettings{nullptr}, layoutsSettings{nullptr}, downloadSettings{nullptr},
      cardDatabasePath{QString("%1/cards.xml").arg(CARDDB_DATADIR)},
      customCardDatabasePath{QString("%1/customsets/").arg(CARDDB_DATADIR)},
      spoilerDatabasePath{QString("%1/spoiler.xml").arg(CARDDB_DATADIR)},
      tokenDatabasePath{QString("%1/tokens.xml").arg(CARDDB_DATADIR)}
{
}
void SettingsCache::setUseTearOffMenus(bool /* _useTearOffMenus */)
{
}
void SettingsCache::setCardViewInitialRowsMax(int /* _cardViewInitialRowsMax */)
{
}
void SettingsCache::setCardViewExpandedRowsMax(int /* value */)
{
}
void SettingsCache::setCloseEmptyCardView(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setFocusCardViewSearchBar(QT_STATE_CHANGED_T /* value */)
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
void SettingsCache::setFiltersPath(const QString & /*_filtersPath */)
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
void SettingsCache::setTabVisualDeckStorageOpen(bool /*value*/)
{
}
void SettingsCache::setTabServerOpen(bool /*value*/)
{
}
void SettingsCache::setTabAccountOpen(bool /*value*/)
{
}
void SettingsCache::setTabDeckStorageOpen(bool /*value*/)
{
}
void SettingsCache::setTabReplaysOpen(bool /*value*/)
{
}
void SettingsCache::setTabAdminOpen(bool /*value*/)
{
}
void SettingsCache::setTabLogOpen(bool /*value*/)
{
}
void SettingsCache::setPicDownload(QT_STATE_CHANGED_T /* _picDownload */)
{
}
void SettingsCache::setShowStatusBar(bool /* value */)
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
void SettingsCache::setClickPlaysAllSelected(QT_STATE_CHANGED_T /* _clickPlaysAllSelected */)
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
void SettingsCache::setShowShortcuts(QT_STATE_CHANGED_T /* _showShortcuts */)
{
}
void SettingsCache::setDisplayCardNames(QT_STATE_CHANGED_T /* _displayCardNames */)
{
}
void SettingsCache::setOverrideAllCardArtWithPersonalPreference(QT_STATE_CHANGED_T /* _overrideAllCardArt */)
{
}
void SettingsCache::setBumpSetsWithCardsInDeckToTop(QT_STATE_CHANGED_T /* _bumpSetsWithCardsInDeckToTop */)
{
}
void SettingsCache::setPrintingSelectorSortOrder(int /* _printingSelectorSortOrder */)
{
}
void SettingsCache::setPrintingSelectorCardSize(int /* _printingSelectorCardSize */)
{
}
void SettingsCache::setIncludeRebalancedCards(bool /* _includeRebalancedCards */)
{
}
void SettingsCache::setPrintingSelectorNavigationButtonsVisible(QT_STATE_CHANGED_T /* _navigationButtonsVisible */)
{
}
void SettingsCache::setDeckEditorBannerCardComboBoxVisible(
    QT_STATE_CHANGED_T /* _deckEditorBannerCardComboBoxVisible */)
{
}
void SettingsCache::setDeckEditorTagsWidgetVisible(QT_STATE_CHANGED_T /* _deckEditorTagsWidgetVisible */)
{
}
void SettingsCache::setVisualDeckStorageSortingOrder(int /* _visualDeckStorageSortingOrder */)
{
}
void SettingsCache::setVisualDeckStorageShowFolders(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setVisualDeckStorageShowTagFilter(QT_STATE_CHANGED_T /* _showTags */)
{
}
void SettingsCache::setVisualDeckStorageDefaultTagsList(QStringList /* _defaultTagsList */)
{
}
void SettingsCache::setVisualDeckStorageSearchFolderNames(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setVisualDeckStorageShowBannerCardComboBox(QT_STATE_CHANGED_T /* _showBannerCardComboBox */)
{
}
void SettingsCache::setVisualDeckStorageShowTagsOnDeckPreviews(QT_STATE_CHANGED_T /* _showTags */)
{
}
void SettingsCache::setVisualDeckStorageCardSize(int /* _visualDeckStorageCardSize */)
{
}
void SettingsCache::setVisualDeckStorageDrawUnusedColorIdentities(
    QT_STATE_CHANGED_T /* _visualDeckStorageDrawUnusedColorIdentities */)
{
}
void SettingsCache::setVisualDeckStorageUnusedColorIdentitiesOpacity(
    int /* _visualDeckStorageUnusedColorIdentitiesOpacity */)
{
}
void SettingsCache::setVisualDeckStorageTooltipType(int /* value */)
{
}
void SettingsCache::setVisualDeckStoragePromptForConversion(bool /* _visualDeckStoragePromptForConversion */)
{
}
void SettingsCache::setVisualDeckStorageAlwaysConvert(bool /* _visualDeckStorageAlwaysConvert */)
{
}
void SettingsCache::setVisualDeckStorageInGame(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setVisualDeckStorageSelectionAnimation(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setDefaultDeckEditorType(int /* value */)
{
}
void SettingsCache::setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(QT_STATE_CHANGED_T /* _enabled */)
{
}
void SettingsCache::setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int /* _amount */)
{
}
void SettingsCache::setVisualDeckEditorSampleHandSize(int /* _amount */)
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
void SettingsCache::setAutoRotateSidewaysLayoutCards(QT_STATE_CHANGED_T /* _autoRotateSidewaysLayoutCards */)
{
}
void SettingsCache::setOpenDeckInNewTab(QT_STATE_CHANGED_T /* _openDeckInNewTab */)
{
}
void SettingsCache::setRewindBufferingMs(int /* _rewindBufferingMs */)
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
void SettingsCache::setDefaultStartingLifeTotal(const int /* _startingLifeTotal */)
{
}
void SettingsCache::setShareDecklistsOnLoad(const bool /* _shareDecklistsOnLoad */)
{
}
void SettingsCache::setRememberGameSettings(const bool /* _rememberGameSettings */)
{
}
void SettingsCache::setCheckUpdatesOnStartup(QT_STATE_CHANGED_T /* value */)
{
}
void SettingsCache::setStartupCardUpdateCheckPromptForUpdate(bool /* value */)
{
}
void SettingsCache::setStartupCardUpdateCheckAlwaysUpdate(bool /* value */)
{
}
void SettingsCache::setCardUpdateCheckInterval(int /* value */)
{
}
void SettingsCache::setLastCardUpdateCheck(QDate /* value */)
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
void SettingsCache::setUpdateReleaseChannelIndex(int /* value */)
{
}
void SettingsCache::setMaxFontSize(int /* _max */)
{
}
void SettingsCache::setRoundCardCorners(bool /* _roundCardCorners */)
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
