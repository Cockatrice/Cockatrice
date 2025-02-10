#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include "../utility/macros.h"
#include "card_database_settings.h"
#include "card_override_settings.h"
#include "debug_settings.h"
#include "download_settings.h"
#include "game_filters_settings.h"
#include "layouts_settings.h"
#include "message_settings.h"
#include "recents_settings.h"
#include "servers_settings.h"
#include "shortcuts_settings.h"

#include <QLoggingCategory>
#include <QObject>
#include <QSize>
#include <QStringList>

inline Q_LOGGING_CATEGORY(SettingsCacheLog, "settings_cache");

class ReleaseChannel;

// In MB (Increments of 64)
#define PIXMAPCACHE_SIZE_DEFAULT 2048
#define PIXMAPCACHE_SIZE_MIN 64
#define PIXMAPCACHE_SIZE_MAX 4096

// In MB
constexpr int NETWORK_CACHE_SIZE_DEFAULT = 1024 * 4; // 4 GB
constexpr int NETWORK_CACHE_SIZE_MIN = 1;            // 1 MB
constexpr int NETWORK_CACHE_SIZE_MAX = 1024 * 1024;  // 1 TB

// In Days
#define NETWORK_REDIRECT_CACHE_TTL_DEFAULT 30
#define NETWORK_REDIRECT_CACHE_TTL_MIN 1
#define NETWORK_REDIRECT_CACHE_TTL_MAX 90

#define DEFAULT_LANG_NAME "English"
#define CLIENT_INFO_NOT_SET "notset"

#define DEFAULT_FONT_SIZE 12

class QSettings;

class SettingsCache : public QObject
{
    Q_OBJECT
signals:
    void langChanged();
    void picsPathChanged();
    void cardDatabasePathChanged();
    void themeChanged();
    void picDownloadChanged();
    void displayCardNamesChanged();
    void overrideAllCardArtWithPersonalPreferenceChanged();
    void bumpSetsWithCardsInDeckToTopChanged();
    void printingSelectorSortOrderChanged();
    void printingSelectorCardSizeChanged();
    void printingSelectorSortOptionsVisibleChanged();
    void printingSelectorSearchBarVisibleChanged();
    void printingSelectorCardSizeSliderVisibleChanged();
    void printingSelectorNavigationButtonsVisibleChanged();
    void visualDeckStorageShowTagFilterChanged(bool _visible);
    void visualDeckStorageShowTagsOnDeckPreviewsChanged(bool _visible);
    void visualDeckStorageCardSizeChanged();
    void visualDeckStorageInGameChanged(bool enabled);
    void horizontalHandChanged();
    void handJustificationChanged();
    void invertVerticalCoordinateChanged();
    void minPlayersForMultiColumnLayoutChanged();
    void soundEnabledChanged();
    void soundThemeChanged();
    void ignoreUnregisteredUsersChanged();
    void ignoreUnregisteredUserMessagesChanged();
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void networkCacheSizeChanged(int newSizeInMBs);
    void redirectCacheTtlChanged(int newTtl);
    void masterVolumeChanged(int value);
    void chatMentionCompleterChanged();
    void downloadSpoilerTimeIndexChanged();
    void downloadSpoilerStatusChanged();
    void useTearOffMenusChanged(bool state);

private:
    QSettings *settings;
    ShortcutsSettings *shortcutsSettings;
    CardDatabaseSettings *cardDatabaseSettings;
    ServersSettings *serversSettings;
    MessageSettings *messageSettings;
    GameFiltersSettings *gameFiltersSettings;
    LayoutsSettings *layoutsSettings;
    DownloadSettings *downloadSettings;
    RecentsSettings *recentsSettings;
    CardOverrideSettings *cardOverrideSettings;
    DebugSettings *debugSettings;

    QByteArray mainWindowGeometry;
    QByteArray tokenDialogGeometry;
    QByteArray setsDialogGeometry;
    QString lang;
    QString deckPath, replaysPath, picsPath, redirectCachePath, customPicsPath, cardDatabasePath,
        customCardDatabasePath, themesPath, spoilerDatabasePath, tokenDatabasePath, themeName;
    bool tabVisualDeckStorageOpen, tabServerOpen, tabAccountOpen, tabDeckStorageOpen, tabReplaysOpen, tabAdminOpen,
        tabLogOpen;
    bool checkUpdatesOnStartup;
    bool notifyAboutUpdates;
    bool notifyAboutNewVersion;
    bool showTipsOnStartup;
    QList<int> seenTips;
    bool mbDownloadSpoilers;
    int updateReleaseChannel;
    int maxFontSize;
    bool picDownload;
    bool notificationsEnabled;
    bool spectatorNotificationsEnabled;
    bool buddyConnectNotificationsEnabled;
    bool doubleClickToPlay;
    bool clickPlaysAllSelected;
    bool playToStack;
    int startingHandSize;
    bool annotateTokens;
    QByteArray tabGameSplitterSizes;
    bool showShortcuts;
    bool displayCardNames;
    bool overrideAllCardArtWithPersonalPreference;
    bool bumpSetsWithCardsInDeckToTop;
    int printingSelectorSortOrder;
    int printingSelectorCardSize;
    bool printingSelectorSortOptionsVisible;
    bool printingSelectorSearchBarVisible;
    bool printingSelectorCardSizeSliderVisible;
    bool printingSelectorNavigationButtonsVisible;
    int visualDeckStorageSortingOrder;
    bool visualDeckStorageShowFolders;
    bool visualDeckStorageShowTagsOnDeckPreviews;
    bool visualDeckStorageShowTagFilter;
    bool visualDeckStorageShowCardSizeSlider;
    int visualDeckStorageCardSize;
    bool visualDeckStorageDrawUnusedColorIdentities;
    int visualDeckStorageUnusedColorIdentitiesOpacity;
    bool visualDeckStoragePromptForConversion;
    bool visualDeckStorageAlwaysConvert;
    bool visualDeckStorageInGame;
    bool horizontalHand;
    bool invertVerticalCoordinate;
    int minPlayersForMultiColumnLayout;
    bool tapAnimation;
    bool autoRotateSidewaysLayoutCards;
    bool openDeckInNewTab;
    int rewindBufferingMs;
    bool chatMention;
    bool chatMentionCompleter;
    QString chatMentionColor;
    QString chatHighlightColor;
    bool chatMentionForeground;
    bool chatHighlightForeground;
    int zoneViewSortByIndex, zoneViewGroupByIndex;
    bool zoneViewPileView;
    bool soundEnabled;
    QString soundThemeName;
    bool ignoreUnregisteredUsers;
    bool ignoreUnregisteredUserMessages;
    QString picUrl;
    QString picUrlFallback;
    QString clientID;
    QString clientVersion;
    QString knownMissingFeatures;
    bool useTearOffMenus;
    int cardViewInitialRowsMax;
    bool closeEmptyCardView;
    int pixmapCacheSize;
    int networkCacheSize;
    int redirectCacheTtl;
    bool scaleCards;
    int verticalCardOverlapPercent;
    bool showMessagePopups;
    bool showMentionPopups;
    bool roomHistory;
    bool leftJustified;
    int masterVolume;
    int cardInfoViewMode;
    QString highlightWords;
    QString gameDescription;
    int maxPlayers;
    QString gameTypes;
    bool onlyBuddies;
    bool onlyRegistered;
    bool spectatorsAllowed;
    bool spectatorsNeedPassword;
    bool spectatorsCanTalk;
    bool spectatorsCanSeeEverything;
    bool createGameAsSpectator;
    int defaultStartingLifeTotal;
    int keepalive;
    int timeout;
    void translateLegacySettings();
    QString getSafeConfigPath(QString configEntry, QString defaultPath) const;
    QString getSafeConfigFilePath(QString configEntry, QString defaultPath) const;
    void loadPaths();
    bool rememberGameSettings;
    QList<ReleaseChannel *> releaseChannels;
    bool isPortableBuild;

public:
    SettingsCache();
    QString getDataPath();
    QString getSettingsPath();
    QString getCachePath() const;
    QString getNetworkCachePath() const;
    const QByteArray &getMainWindowGeometry() const
    {
        return mainWindowGeometry;
    }
    const QByteArray &getTokenDialogGeometry() const
    {
        return tokenDialogGeometry;
    }
    const QByteArray &getSetsDialogGeometry() const
    {
        return setsDialogGeometry;
    }
    QString getLang() const
    {
        return lang;
    }
    QString getDeckPath() const
    {
        return deckPath;
    }
    QString getReplaysPath() const
    {
        return replaysPath;
    }
    QString getThemesPath() const
    {
        return themesPath;
    }
    QString getPicsPath() const
    {
        return picsPath;
    }
    QString getRedirectCachePath() const
    {
        return redirectCachePath;
    }
    QString getCustomPicsPath() const
    {
        return customPicsPath;
    }
    QString getCustomCardDatabasePath() const
    {
        return customCardDatabasePath;
    }
    QString getCardDatabasePath() const
    {
        return cardDatabasePath;
    }
    QString getSpoilerCardDatabasePath() const
    {
        return spoilerDatabasePath;
    }
    QString getTokenDatabasePath() const
    {
        return tokenDatabasePath;
    }
    QString getThemeName() const
    {
        return themeName;
    }
    bool getTabVisualDeckStorageOpen() const
    {
        return tabVisualDeckStorageOpen;
    }
    bool getTabServerOpen() const
    {
        return tabServerOpen;
    }
    bool getTabAccountOpen() const
    {
        return tabAccountOpen;
    }
    bool getTabDeckStorageOpen() const
    {
        return tabDeckStorageOpen;
    }
    bool getTabReplaysOpen() const
    {
        return tabReplaysOpen;
    }
    bool getTabAdminOpen() const
    {
        return tabAdminOpen;
    }
    bool getTabLogOpen() const
    {
        return tabLogOpen;
    }
    QString getChatMentionColor() const
    {
        return chatMentionColor;
    }
    QString getChatHighlightColor() const
    {
        return chatHighlightColor;
    }
    bool getPicDownload() const
    {
        return picDownload;
    }
    bool getNotificationsEnabled() const
    {
        return notificationsEnabled;
    }
    bool getSpectatorNotificationsEnabled() const
    {
        return spectatorNotificationsEnabled;
    }
    bool getBuddyConnectNotificationsEnabled() const
    {
        return buddyConnectNotificationsEnabled;
    }
    bool getCheckUpdatesOnStartup() const
    {
        return checkUpdatesOnStartup;
    }
    bool getNotifyAboutUpdates() const
    {
        return notifyAboutUpdates;
    }
    bool getNotifyAboutNewVersion() const
    {
        return notifyAboutNewVersion;
    }
    bool getShowTipsOnStartup() const
    {
        return showTipsOnStartup;
    }
    QList<int> getSeenTips() const
    {
        return seenTips;
    }
    int getUpdateReleaseChannelIndex() const
    {
        return updateReleaseChannel;
    }
    ReleaseChannel *getUpdateReleaseChannel() const
    {
        return releaseChannels.at(qMax(0, updateReleaseChannel));
    }
    QList<ReleaseChannel *> getUpdateReleaseChannels() const
    {
        return releaseChannels;
    }

    bool getDoubleClickToPlay() const
    {
        return doubleClickToPlay;
    }
    bool getClickPlaysAllSelected() const
    {
        return clickPlaysAllSelected;
    }
    bool getPlayToStack() const
    {
        return playToStack;
    }
    int getStartingHandSize() const
    {
        return startingHandSize;
    }
    bool getAnnotateTokens() const
    {
        return annotateTokens;
    }
    QByteArray getTabGameSplitterSizes() const
    {
        return tabGameSplitterSizes;
    }
    bool getShowShortcuts() const
    {
        return showShortcuts;
    }
    bool getDisplayCardNames() const
    {
        return displayCardNames;
    }
    bool getOverrideAllCardArtWithPersonalPreference() const
    {
        return overrideAllCardArtWithPersonalPreference;
    }
    bool getBumpSetsWithCardsInDeckToTop() const
    {
        return bumpSetsWithCardsInDeckToTop;
    }
    int getPrintingSelectorSortOrder() const
    {
        return printingSelectorSortOrder;
    }
    int getPrintingSelectorCardSize() const
    {
        return printingSelectorCardSize;
    }
    bool getPrintingSelectorSortOptionsVisible() const
    {
        return printingSelectorSortOptionsVisible;
    }
    bool getPrintingSelectorSearchBarVisible() const
    {
        return printingSelectorSearchBarVisible;
    }
    bool getPrintingSelectorCardSizeSliderVisible() const
    {
        return printingSelectorCardSizeSliderVisible;
    }
    bool getPrintingSelectorNavigationButtonsVisible() const
    {
        return printingSelectorNavigationButtonsVisible;
    }
    int getVisualDeckStorageSortingOrder() const
    {
        return visualDeckStorageSortingOrder;
    }
    bool getVisualDeckStorageShowFolders() const
    {
        return visualDeckStorageShowFolders;
    }
    bool getVisualDeckStorageShowTagFilter() const
    {
        return visualDeckStorageShowTagFilter;
    }
    bool getVisualDeckStorageShowTagsOnDeckPreviews() const
    {
        return visualDeckStorageShowTagsOnDeckPreviews;
    }
    bool getVisualDeckStorageShowCardSizeSlider() const
    {
        return visualDeckStorageShowCardSizeSlider;
    }
    int getVisualDeckStorageCardSize() const
    {
        return visualDeckStorageCardSize;
    }
    bool getVisualDeckStorageDrawUnusedColorIdentities() const
    {
        return visualDeckStorageDrawUnusedColorIdentities;
    }
    int getVisualDeckStorageUnusedColorIdentitiesOpacity() const
    {
        return visualDeckStorageUnusedColorIdentitiesOpacity;
    }
    bool getVisualDeckStoragePromptForConversion() const
    {
        return visualDeckStoragePromptForConversion;
    }
    bool getVisualDeckStorageAlwaysConvert() const
    {
        return visualDeckStorageAlwaysConvert;
    }
    bool getVisualDeckStorageInGame() const
    {
        return visualDeckStorageInGame;
    }
    bool getHorizontalHand() const
    {
        return horizontalHand;
    }
    bool getInvertVerticalCoordinate() const
    {
        return invertVerticalCoordinate;
    }
    int getMinPlayersForMultiColumnLayout() const
    {
        return minPlayersForMultiColumnLayout;
    }
    bool getTapAnimation() const
    {
        return tapAnimation;
    }
    bool getAutoRotateSidewaysLayoutCards() const
    {
        return autoRotateSidewaysLayoutCards;
    }
    bool getOpenDeckInNewTab() const
    {
        return openDeckInNewTab;
    }
    int getRewindBufferingMs() const
    {
        return rewindBufferingMs;
    }
    bool getChatMention() const
    {
        return chatMention;
    }
    bool getChatMentionCompleter() const
    {
        return chatMentionCompleter;
    }
    bool getChatMentionForeground() const
    {
        return chatMentionForeground;
    }
    bool getChatHighlightForeground() const
    {
        return chatHighlightForeground;
    }
    /**
     * Currently selected index for the `Group by X` QComboBox
     */
    int getZoneViewGroupByIndex() const
    {
        return zoneViewGroupByIndex;
    }
    /**
     * Currently selected index for the `Sort by X` QComboBox
     */
    int getZoneViewSortByIndex() const
    {
        return zoneViewSortByIndex;
    }
    /**
       Returns if the view should be sorted into pile view.
       @return zoneViewPileView if the view should be sorted into pile view.
     */
    bool getZoneViewPileView() const
    {
        return zoneViewPileView;
    }
    bool getSoundEnabled() const
    {
        return soundEnabled;
    }
    QString getSoundThemeName() const
    {
        return soundThemeName;
    }
    bool getIgnoreUnregisteredUsers() const
    {
        return ignoreUnregisteredUsers;
    }
    bool getIgnoreUnregisteredUserMessages() const
    {
        return ignoreUnregisteredUserMessages;
    }
    int getPixmapCacheSize() const
    {
        return pixmapCacheSize;
    }
    int getNetworkCacheSizeInMB() const
    {
        return networkCacheSize;
    }
    int getRedirectCacheTtl() const
    {
        return redirectCacheTtl;
    }
    bool getScaleCards() const
    {
        return scaleCards;
    }
    int getStackCardOverlapPercent() const
    {
        return verticalCardOverlapPercent;
    }
    bool getShowMessagePopup() const
    {
        return showMessagePopups;
    }
    bool getShowMentionPopup() const
    {
        return showMentionPopups;
    }
    bool getRoomHistory() const
    {
        return roomHistory;
    }
    bool getLeftJustified() const
    {
        return leftJustified;
    }
    int getMasterVolume() const
    {
        return masterVolume;
    }
    int getCardInfoViewMode() const
    {
        return cardInfoViewMode;
    }
    QStringList getCountries() const;
    QString getHighlightWords() const
    {
        return highlightWords;
    }
    QString getGameDescription() const
    {
        return gameDescription;
    }
    int getMaxPlayers() const
    {
        return maxPlayers;
    }
    QString getGameTypes() const
    {
        return gameTypes;
    }
    bool getOnlyBuddies() const
    {
        return onlyBuddies;
    }
    bool getOnlyRegistered() const
    {
        return onlyRegistered;
    }
    bool getSpectatorsAllowed() const
    {
        return spectatorsAllowed;
    }
    bool getSpectatorsNeedPassword() const
    {
        return spectatorsNeedPassword;
    }
    bool getSpectatorsCanTalk() const
    {
        return spectatorsCanTalk;
    }
    bool getSpectatorsCanSeeEverything() const
    {
        return spectatorsCanSeeEverything;
    }
    int getDefaultStartingLifeTotal() const
    {
        return defaultStartingLifeTotal;
    }
    bool getCreateGameAsSpectator() const
    {
        return createGameAsSpectator;
    }
    bool getRememberGameSettings() const
    {
        return rememberGameSettings;
    }
    int getKeepAlive() const
    {
        return keepalive;
    }
    int getTimeOut() const
    {
        return timeout;
    }
    int getMaxFontSize() const
    {
        return maxFontSize;
    }
    void setClientID(const QString &clientID);
    void setClientVersion(const QString &clientVersion);
    void setKnownMissingFeatures(const QString &_knownMissingFeatures);
    void setUseTearOffMenus(bool _useTearOffMenus);
    void setCardViewInitialRowsMax(int _cardViewInitialRowsMax);
    void setCloseEmptyCardView(QT_STATE_CHANGED_T value);
    QString getClientID()
    {
        return clientID;
    }
    QString getClientVersion()
    {
        return clientVersion;
    }
    QString getKnownMissingFeatures()
    {
        return knownMissingFeatures;
    }
    bool getUseTearOffMenus()
    {
        return useTearOffMenus;
    }
    int getCardViewInitialRowsMax() const
    {
        return cardViewInitialRowsMax;
    }
    bool getCloseEmptyCardView() const
    {
        return closeEmptyCardView;
    }
    ShortcutsSettings &shortcuts() const
    {
        return *shortcutsSettings;
    }
    CardDatabaseSettings &cardDatabase() const
    {
        return *cardDatabaseSettings;
    }
    ServersSettings &servers() const
    {
        return *serversSettings;
    }
    MessageSettings &messages() const
    {
        return *messageSettings;
    }
    GameFiltersSettings &gameFilters() const
    {
        return *gameFiltersSettings;
    }
    LayoutsSettings &layouts() const
    {
        return *layoutsSettings;
    }
    DownloadSettings &downloads() const
    {
        return *downloadSettings;
    }
    RecentsSettings &recents() const
    {
        return *recentsSettings;
    }
    CardOverrideSettings &cardOverrides() const
    {
        return *cardOverrideSettings;
    }
    DebugSettings &debug() const
    {
        return *debugSettings;
    }
    bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }
    bool getDownloadSpoilersStatus() const
    {
        return mbDownloadSpoilers;
    }

    static SettingsCache &instance();
    void resetPaths();

public slots:
    void setDownloadSpoilerStatus(bool _spoilerStatus);

    void setMainWindowGeometry(const QByteArray &_mainWindowGeometry);
    void setTokenDialogGeometry(const QByteArray &_tokenDialog);
    void setSetsDialogGeometry(const QByteArray &_setsDialog);
    void setLang(const QString &_lang);
    void setShowTipsOnStartup(bool _showTipsOnStartup);
    void setSeenTips(const QList<int> &_seenTips);
    void setDeckPath(const QString &_deckPath);
    void setReplaysPath(const QString &_replaysPath);
    void setThemesPath(const QString &_themesPath);
    void setCustomCardDatabasePath(const QString &_customCardDatabasePath);
    void setPicsPath(const QString &_picsPath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setSpoilerDatabasePath(const QString &_spoilerDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);
    void setThemeName(const QString &_themeName);
    void setTabVisualDeckStorageOpen(bool value);
    void setTabServerOpen(bool value);
    void setTabAccountOpen(bool value);
    void setTabDeckStorageOpen(bool value);
    void setTabReplaysOpen(bool value);
    void setTabAdminOpen(bool value);
    void setTabLogOpen(bool value);
    void setChatMentionColor(const QString &_chatMentionColor);
    void setChatHighlightColor(const QString &_chatHighlightColor);
    void setPicDownload(QT_STATE_CHANGED_T _picDownload);
    void setNotificationsEnabled(QT_STATE_CHANGED_T _notificationsEnabled);
    void setSpectatorNotificationsEnabled(QT_STATE_CHANGED_T _spectatorNotificationsEnabled);
    void setBuddyConnectNotificationsEnabled(QT_STATE_CHANGED_T _buddyConnectNotificationsEnabled);
    void setDoubleClickToPlay(QT_STATE_CHANGED_T _doubleClickToPlay);
    void setClickPlaysAllSelected(QT_STATE_CHANGED_T _clickPlaysAllSelected);
    void setPlayToStack(QT_STATE_CHANGED_T _playToStack);
    void setStartingHandSize(int _startingHandSize);
    void setAnnotateTokens(QT_STATE_CHANGED_T _annotateTokens);
    void setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes);
    void setShowShortcuts(QT_STATE_CHANGED_T _showShortcuts);
    void setDisplayCardNames(QT_STATE_CHANGED_T _displayCardNames);
    void setOverrideAllCardArtWithPersonalPreference(QT_STATE_CHANGED_T _overrideAllCardArt);
    void setBumpSetsWithCardsInDeckToTop(QT_STATE_CHANGED_T _bumpSetsWithCardsInDeckToTop);
    void setPrintingSelectorSortOrder(int _printingSelectorSortOrder);
    void setPrintingSelectorCardSize(int _printingSelectorCardSize);
    void setPrintingSelectorSortOptionsVisible(QT_STATE_CHANGED_T _sortOptionsVisible);
    void setPrintingSelectorSearchBarVisible(QT_STATE_CHANGED_T _searchBarVisible);
    void setPrintingSelectorCardSizeSliderVisible(QT_STATE_CHANGED_T _cardSizeSliderVisible);
    void setPrintingSelectorNavigationButtonsVisible(QT_STATE_CHANGED_T _navigationButtonsVisible);
    void setVisualDeckStorageSortingOrder(int _visualDeckStorageSortingOrder);
    void setVisualDeckStorageShowFolders(QT_STATE_CHANGED_T value);
    void setVisualDeckStorageShowTagFilter(QT_STATE_CHANGED_T _showTags);
    void setVisualDeckStorageShowTagsOnDeckPreviews(QT_STATE_CHANGED_T _showTags);
    void setVisualDeckStorageShowCardSizeSlider(QT_STATE_CHANGED_T _showCardSizeSlider);
    void setVisualDeckStorageCardSize(int _visualDeckStorageCardSize);
    void setVisualDeckStorageDrawUnusedColorIdentities(QT_STATE_CHANGED_T _visualDeckStorageDrawUnusedColorIdentities);
    void setVisualDeckStorageUnusedColorIdentitiesOpacity(int _visualDeckStorageUnusedColorIdentitiesOpacity);
    void setVisualDeckStoragePromptForConversion(QT_STATE_CHANGED_T _visualDeckStoragePromptForConversion);
    void setVisualDeckStorageAlwaysConvert(QT_STATE_CHANGED_T _visualDeckStorageAlwaysConvert);
    void setVisualDeckStorageInGame(QT_STATE_CHANGED_T value);
    void setHorizontalHand(QT_STATE_CHANGED_T _horizontalHand);
    void setInvertVerticalCoordinate(QT_STATE_CHANGED_T _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setTapAnimation(QT_STATE_CHANGED_T _tapAnimation);
    void setAutoRotateSidewaysLayoutCards(QT_STATE_CHANGED_T _autoRotateSidewaysLayoutCards);
    void setOpenDeckInNewTab(QT_STATE_CHANGED_T _openDeckInNewTab);
    void setRewindBufferingMs(int _rewindBufferingMs);
    void setChatMention(QT_STATE_CHANGED_T _chatMention);
    void setChatMentionCompleter(QT_STATE_CHANGED_T _chatMentionCompleter);
    void setChatMentionForeground(QT_STATE_CHANGED_T _chatMentionForeground);
    void setChatHighlightForeground(QT_STATE_CHANGED_T _chatHighlightForeground);
    void setZoneViewGroupByIndex(const int _zoneViewGroupByIndex);
    void setZoneViewSortByIndex(const int _zoneViewSortByIndex);
    void setZoneViewPileView(QT_STATE_CHANGED_T _zoneViewPileView);
    void setSoundEnabled(QT_STATE_CHANGED_T _soundEnabled);
    void setSoundThemeName(const QString &_soundThemeName);
    void setIgnoreUnregisteredUsers(QT_STATE_CHANGED_T _ignoreUnregisteredUsers);
    void setIgnoreUnregisteredUserMessages(QT_STATE_CHANGED_T _ignoreUnregisteredUserMessages);
    void setPixmapCacheSize(const int _pixmapCacheSize);
    void setNetworkCacheSizeInMB(const int _networkCacheSize);
    void setNetworkRedirectCacheTtl(const int _redirectCacheTtl);
    void setCardScaling(const QT_STATE_CHANGED_T _scaleCards);
    void setStackCardOverlapPercent(const int _verticalCardOverlapPercent);
    void setShowMessagePopups(const QT_STATE_CHANGED_T _showMessagePopups);
    void setShowMentionPopups(const QT_STATE_CHANGED_T _showMentionPopups);
    void setRoomHistory(const QT_STATE_CHANGED_T _roomHistory);
    void setLeftJustified(const QT_STATE_CHANGED_T _leftJustified);
    void setMasterVolume(const int _masterVolume);
    void setCardInfoViewMode(const int _viewMode);
    void setHighlightWords(const QString &_highlightWords);
    void setGameDescription(const QString _gameDescription);
    void setMaxPlayers(const int _maxPlayers);
    void setGameTypes(const QString _gameTypes);
    void setOnlyBuddies(const bool _onlyBuddies);
    void setOnlyRegistered(const bool _onlyRegistered);
    void setSpectatorsAllowed(const bool _spectatorsAllowed);
    void setSpectatorsNeedPassword(const bool _spectatorsNeedPassword);
    void setSpectatorsCanTalk(const bool _spectatorsCanTalk);
    void setSpectatorsCanSeeEverything(const bool _spectatorsCanSeeEverything);
    void setCreateGameAsSpectator(const bool _createGameAsSpectator);
    void setDefaultStartingLifeTotal(const int _defaultStartingLifeTotal);
    void setRememberGameSettings(const bool _rememberGameSettings);
    void setCheckUpdatesOnStartup(QT_STATE_CHANGED_T value);
    void setNotifyAboutUpdate(QT_STATE_CHANGED_T _notifyaboutupdate);
    void setNotifyAboutNewVersion(QT_STATE_CHANGED_T _notifyaboutnewversion);
    void setUpdateReleaseChannelIndex(int value);
    void setMaxFontSize(int _max);
};

#endif
