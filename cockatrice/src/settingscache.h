#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include "settings/carddatabasesettings.h"
#include "settings/downloadsettings.h"
#include "settings/gamefilterssettings.h"
#include "settings/layoutssettings.h"
#include "settings/messagesettings.h"
#include "settings/serverssettings.h"
#include "shortcutssettings.h"
#include <QObject>
#include <QSize>
#include <QStringList>

class ReleaseChannel;

// size should be a multiple of 64
#define PIXMAPCACHE_SIZE_DEFAULT 2047
#define PIXMAPCACHE_SIZE_MIN 64
#define PIXMAPCACHE_SIZE_MAX 2047

#define DEFAULT_LANG_CODE "en"
#define DEFAULT_LANG_NAME "English"

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
    void horizontalHandChanged();
    void handJustificationChanged();
    void invertVerticalCoordinateChanged();
    void minPlayersForMultiColumnLayoutChanged();
    void soundEnabledChanged();
    void soundThemeChanged();
    void ignoreUnregisteredUsersChanged();
    void ignoreUnregisteredUserMessagesChanged();
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void masterVolumeChanged(int value);
    void chatMentionCompleterChanged();
    void downloadSpoilerTimeIndexChanged();
    void downloadSpoilerStatusChanged();

private:
    QSettings *settings;
    ShortcutsSettings *shortcutsSettings;
    CardDatabaseSettings *cardDatabaseSettings;
    ServersSettings *serversSettings;
    MessageSettings *messageSettings;
    GameFiltersSettings *gameFiltersSettings;
    LayoutsSettings *layoutsSettings;
    DownloadSettings *downloadSettings;

    QByteArray mainWindowGeometry;
    QByteArray tokenDialogGeometry;
    QString lang;
    QString deckPath, replaysPath, picsPath, customPicsPath, cardDatabasePath, customCardDatabasePath,
        spoilerDatabasePath, tokenDatabasePath, themeName;
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
    bool doubleClickToPlay;
    bool playToStack;
    bool annotateTokens;
    QByteArray tabGameSplitterSizes;
    bool displayCardNames;
    bool horizontalHand;
    bool invertVerticalCoordinate;
    int minPlayersForMultiColumnLayout;
    bool tapAnimation;
    bool chatMention;
    bool chatMentionCompleter;
    QString chatMentionColor;
    QString chatHighlightColor;
    bool chatMentionForeground;
    bool chatHighlightForeground;
    bool zoneViewSortByName, zoneViewSortByType, zoneViewPileView;
    bool soundEnabled;
    QString soundThemeName;
    bool ignoreUnregisteredUsers;
    bool ignoreUnregisteredUserMessages;
    QString picUrl;
    QString picUrlFallback;
    QString clientID;
    QString clientVersion;
    QString knownMissingFeatures;
    int pixmapCacheSize;
    bool scaleCards;
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
    int keepalive;
    void translateLegacySettings();
    QString getSafeConfigPath(QString configEntry, QString defaultPath) const;
    QString getSafeConfigFilePath(QString configEntry, QString defaultPath) const;
    bool rememberGameSettings;
    QList<ReleaseChannel *> releaseChannels;
    bool isPortableBuild;

public:
    SettingsCache();
    QString getDataPath();
    QString getSettingsPath();
    const QByteArray &getMainWindowGeometry() const
    {
        return mainWindowGeometry;
    }
    const QByteArray &getTokenDialogGeometry() const
    {
        return tokenDialogGeometry;
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
    QString getPicsPath() const
    {
        return picsPath;
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
    ReleaseChannel *getUpdateReleaseChannel() const
    {
        return releaseChannels.at(updateReleaseChannel);
    }
    QList<ReleaseChannel *> getUpdateReleaseChannels() const
    {
        return releaseChannels;
    }

    bool getDoubleClickToPlay() const
    {
        return doubleClickToPlay;
    }
    bool getPlayToStack() const
    {
        return playToStack;
    }
    bool getAnnotateTokens() const
    {
        return annotateTokens;
    }
    QByteArray getTabGameSplitterSizes() const
    {
        return tabGameSplitterSizes;
    }
    bool getDisplayCardNames() const
    {
        return displayCardNames;
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
    bool getZoneViewSortByName() const
    {
        return zoneViewSortByName;
    }
    bool getZoneViewSortByType() const
    {
        return zoneViewSortByType;
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
    bool getScaleCards() const
    {
        return scaleCards;
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
    bool getRememberGameSettings() const
    {
        return rememberGameSettings;
    }
    int getKeepAlive() const
    {
        return keepalive;
    }
    int getMaxFontSize() const
    {
        return maxFontSize;
    }
    void setClientID(QString clientID);
    void setClientVersion(QString clientVersion);
    void setKnownMissingFeatures(QString _knownMissingFeatures);
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
    bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }
    bool getDownloadSpoilersStatus() const
    {
        return mbDownloadSpoilers;
    }
public slots:
    void setDownloadSpoilerStatus(bool _spoilerStatus);

    void setMainWindowGeometry(const QByteArray &_mainWindowGeometry);
    void setTokenDialogGeometry(const QByteArray &_tokenDialog);
    void setLang(const QString &_lang);
    void setShowTipsOnStartup(bool _showTipsOnStartup);
    void setSeenTips(const QList<int> &_seenTips);
    void setDeckPath(const QString &_deckPath);
    void setReplaysPath(const QString &_replaysPath);
    void setPicsPath(const QString &_picsPath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setSpoilerDatabasePath(const QString &_spoilerDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);
    void setThemeName(const QString &_themeName);
    void setChatMentionColor(const QString &_chatMentionColor);
    void setChatHighlightColor(const QString &_chatHighlightColor);
    void setPicDownload(int _picDownload);
    void setNotificationsEnabled(int _notificationsEnabled);
    void setSpectatorNotificationsEnabled(int _spectatorNotificationsEnabled);
    void setDoubleClickToPlay(int _doubleClickToPlay);
    void setPlayToStack(int _playToStack);
    void setAnnotateTokens(int _annotateTokens);
    void setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes);
    void setDisplayCardNames(int _displayCardNames);
    void setHorizontalHand(int _horizontalHand);
    void setInvertVerticalCoordinate(int _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setTapAnimation(int _tapAnimation);
    void setChatMention(int _chatMention);
    void setChatMentionCompleter(int _chatMentionCompleter);
    void setChatMentionForeground(int _chatMentionForeground);
    void setChatHighlightForeground(int _chatHighlightForeground);
    void setZoneViewSortByName(int _zoneViewSortByName);
    void setZoneViewSortByType(int _zoneViewSortByType);
    void setZoneViewPileView(int _zoneViewPileView);
    void setSoundEnabled(int _soundEnabled);
    void setSoundThemeName(const QString &_soundThemeName);
    void setIgnoreUnregisteredUsers(int _ignoreUnregisteredUsers);
    void setIgnoreUnregisteredUserMessages(int _ignoreUnregisteredUserMessages);
    void setPixmapCacheSize(const int _pixmapCacheSize);
    void setCardScaling(const int _scaleCards);
    void setShowMessagePopups(const int _showMessagePopups);
    void setShowMentionPopups(const int _showMentionPopups);
    void setRoomHistory(const int _roomHistory);
    void setLeftJustified(const int _leftJustified);
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
    void setRememberGameSettings(const bool _rememberGameSettings);
    void setNotifyAboutUpdate(int _notifyaboutupdate);
    void setNotifyAboutNewVersion(int _notifyaboutnewversion);
    void setUpdateReleaseChannel(int _updateReleaseChannel);
    void setMaxFontSize(int _max);
};

extern SettingsCache *settingsCache;

#endif
