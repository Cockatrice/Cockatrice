#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include <QObject>

#define PIC_URL_DEFAULT "http://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!cardid!&type=card"
#define PIC_URL_FALLBACK "http://mtgimage.com/set/!setcode!/!name!.jpg"
#define PIC_URL_HQ_DEFAULT "http://mtgimage.com/multiverseid/!cardid!.jpg"
#define PIC_URL_HQ_FALLBACK "http://mtgimage.com/set/!setcode!/!name!.jpg"
// size should be a multiple of 64
#define PIXMAPCACHE_SIZE_DEFAULT 2047
#define PIXMAPCACHE_SIZE_MIN 64
#define PIXMAPCACHE_SIZE_MAX 2047

class QSettings;

class SettingsCache : public QObject {
    Q_OBJECT
signals:
    void langChanged();
    void picsPathChanged();
    void cardDatabasePathChanged();
    void tokenDatabasePathChanged();
    void handBgPathChanged();
    void stackBgPathChanged();
    void tableBgPathChanged();
    void playerBgPathChanged();
    void cardBackPicturePathChanged();
    void picDownloadChanged();
    void picDownloadHqChanged();
    void displayCardNamesChanged();
    void horizontalHandChanged();
    void invertVerticalCoordinateChanged();
    void minPlayersForMultiColumnLayoutChanged();
    void soundEnabledChanged();
    void soundPathChanged();
    void priceTagFeatureChanged(int enabled);
    void ignoreUnregisteredUsersChanged();
    void ignoreUnregisteredUserMessagesChanged();
    void pixmapCacheSizeChanged(int newSizeInMBs);
private:
    QSettings *settings;

    QByteArray mainWindowGeometry;
    QString lang;
    QString deckPath, replaysPath, picsPath, cardDatabasePath, tokenDatabasePath;
    QString handBgPath, stackBgPath, tableBgPath, playerBgPath, cardBackPicturePath;
    bool picDownload;
    bool picDownloadHq;
    bool notificationsEnabled;
    bool spectatorNotificationsEnabled;
    bool doubleClickToPlay;
    bool playToStack;
    int cardInfoMinimized;
    QByteArray tabGameSplitterSizes;
    bool displayCardNames;
    bool horizontalHand;
    bool invertVerticalCoordinate;
    int minPlayersForMultiColumnLayout;
    bool tapAnimation;
    bool chatMention;
    QString chatMentionColor;
    bool chatMentionForeground;
    bool zoneViewSortByName, zoneViewSortByType, zoneViewPileView;
    bool soundEnabled;
    QString soundPath;
    bool priceTagFeature;
    int priceTagSource;
    bool ignoreUnregisteredUsers;
    bool ignoreUnregisteredUserMessages;
    QString picUrl;
    QString picUrlHq;
    QString picUrlFallback;
    QString picUrlHqFallback;
    bool attemptAutoConnect;
    int pixmapCacheSize;
public:
    SettingsCache();
    const QByteArray &getMainWindowGeometry() const { return mainWindowGeometry; }
    QString getLang() const { return lang; }
    QString getDeckPath() const { return deckPath; }
    QString getReplaysPath() const { return replaysPath; }
    QString getPicsPath() const { return picsPath; }
    QString getCardDatabasePath() const { return cardDatabasePath; }
    QString getTokenDatabasePath() const { return tokenDatabasePath; }
    QString getHandBgPath() const { return handBgPath; }
    QString getStackBgPath() const { return stackBgPath; }
    QString getTableBgPath() const { return tableBgPath; }
    QString getPlayerBgPath() const { return playerBgPath; }
    QString getCardBackPicturePath() const { return cardBackPicturePath; }
    QString getChatMentionColor() const { return chatMentionColor; }
    bool getPicDownload() const { return picDownload; }
    bool getPicDownloadHq() const { return picDownloadHq; }
    bool getNotificationsEnabled() const { return notificationsEnabled; }
    bool getSpectatorNotificationsEnabled() const { return spectatorNotificationsEnabled; }

    bool getDoubleClickToPlay() const { return doubleClickToPlay; }
    bool getPlayToStack() const { return playToStack; }
    int  getCardInfoMinimized() const { return cardInfoMinimized; }
    QByteArray getTabGameSplitterSizes() const { return tabGameSplitterSizes; }
    bool getDisplayCardNames() const { return displayCardNames; }
    bool getHorizontalHand() const { return horizontalHand; }
    bool getInvertVerticalCoordinate() const { return invertVerticalCoordinate; }
    int getMinPlayersForMultiColumnLayout() const { return minPlayersForMultiColumnLayout; }
    bool getTapAnimation() const { return tapAnimation; }
    bool getChatMention()  const { return chatMention; }
    bool getChatMentionForeground() const { return chatMentionForeground; }
    bool getZoneViewSortByName() const { return zoneViewSortByName; }
    bool getZoneViewSortByType() const { return zoneViewSortByType; }
    /**
       Returns if the view should be sorted into pile view.
       @return zoneViewPileView if the view should be sorted into pile view.
     */
    bool getZoneViewPileView() const { return zoneViewPileView; }
    bool getSoundEnabled() const { return soundEnabled; }
    QString getSoundPath() const { return soundPath; }
    bool getPriceTagFeature() const { return priceTagFeature; }
    int getPriceTagSource() const { return priceTagSource; }
    bool getIgnoreUnregisteredUsers() const { return ignoreUnregisteredUsers; }
    bool getIgnoreUnregisteredUserMessages() const { return ignoreUnregisteredUserMessages; }
    QString getPicUrl() const { return picUrl; }
    QString getPicUrlHq() const { return picUrlHq; }
    QString getPicUrlFallback() const { return picUrlFallback; }
    QString getPicUrlHqFallback() const { return picUrlHqFallback; }
    void copyPath(const QString &src, const QString &dst);
    bool getAutoConnect() const { return attemptAutoConnect; }
    int getPixmapCacheSize() const { return pixmapCacheSize; }
public slots:
    void setMainWindowGeometry(const QByteArray &_mainWindowGeometry);
    void setLang(const QString &_lang);
    void setDeckPath(const QString &_deckPath);
    void setReplaysPath(const QString &_replaysPath);
    void setPicsPath(const QString &_picsPath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);
    void setHandBgPath(const QString &_handBgPath);
    void setStackBgPath(const QString &_stackBgPath);
    void setTableBgPath(const QString &_tableBgPath);
    void setPlayerBgPath(const QString &_playerBgPath);
    void setCardBackPicturePath(const QString &_cardBackPicturePath);
    void setChatMentionColor(const QString &_chatMentionColor);
    void setPicDownload(int _picDownload);
    void setPicDownloadHq(int _picDownloadHq);
    void setNotificationsEnabled(int _notificationsEnabled);
    void setSpectatorNotificationsEnabled(int _spectatorNotificationsEnabled);
    void setDoubleClickToPlay(int _doubleClickToPlay);
    void setPlayToStack(int _playToStack);
    void setCardInfoMinimized(int _cardInfoMinimized);
    void setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes);
    void setDisplayCardNames(int _displayCardNames);
    void setHorizontalHand(int _horizontalHand);
    void setInvertVerticalCoordinate(int _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setTapAnimation(int _tapAnimation);
    void setChatMention(int _chatMention);
    void setChatMentionForeground(int _chatMentionForeground);
    void setZoneViewSortByName(int _zoneViewSortByName);
    void setZoneViewSortByType(int _zoneViewSortByType);
    void setZoneViewPileView(int _zoneViewPileView);
    void setSoundEnabled(int _soundEnabled);
    void setSoundPath(const QString &_soundPath);
    void setPriceTagFeature(int _priceTagFeature);
    void setPriceTagSource(int _priceTagSource);
    void setIgnoreUnregisteredUsers(int _ignoreUnregisteredUsers);
    void setIgnoreUnregisteredUserMessages(int _ignoreUnregisteredUserMessages);
    void setPicUrl(const QString &_picUrl);
    void setPicUrlHq(const QString &_picUrlHq);
    void setPicUrlFallback(const QString &_picUrlFallback);
    void setPicUrlHqFallback(const QString &_picUrlHqFallback);
    void setAutoConnect(const bool &_autoConnect);
    void setPixmapCacheSize(const int _pixmapCacheSize);
};

extern SettingsCache *settingsCache;

#endif
