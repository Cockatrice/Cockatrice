#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include <QObject>

#define PIC_URL_DEFAULT "http://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!cardid!&type=card"
#define PIC_URL_HQ_DEFAULT "http://mtgimage.com/multiverseid/!cardid!.jpg"

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
private:
    QSettings *settings;

    QByteArray mainWindowGeometry;
    QString lang;
    QString deckPath, replaysPath, picsPath, cardDatabasePath, tokenDatabasePath;
    QString handBgPath, stackBgPath, tableBgPath, playerBgPath, cardBackPicturePath;
    bool picDownload;
    bool picDownloadHq;
    bool notificationsEnabled;
    bool doubleClickToPlay;
    bool playToStack;
    int cardInfoMinimized;
    QByteArray tabGameSplitterSizes;
    bool displayCardNames;
    bool horizontalHand;
    bool invertVerticalCoordinate;
    int minPlayersForMultiColumnLayout;
    bool tapAnimation;
    bool zoneViewSortByName, zoneViewSortByType;
    bool soundEnabled;
    QString soundPath;
    bool priceTagFeature;
    bool ignoreUnregisteredUsers;
    QString picUrl;
    QString picUrlHq;
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
    bool getPicDownload() const { return picDownload; }
    bool getPicDownloadHq() const { return picDownloadHq; }
    bool getNotificationsEnabled() const { return notificationsEnabled; }
    bool getDoubleClickToPlay() const { return doubleClickToPlay; }
    bool getPlayToStack() const { return playToStack; }
    int  getCardInfoMinimized() const { return cardInfoMinimized; }
    QByteArray getTabGameSplitterSizes() const { return tabGameSplitterSizes; }
    bool getDisplayCardNames() const { return displayCardNames; }
    bool getHorizontalHand() const { return horizontalHand; }
    bool getInvertVerticalCoordinate() const { return invertVerticalCoordinate; }
    int getMinPlayersForMultiColumnLayout() const { return minPlayersForMultiColumnLayout; }
    bool getTapAnimation() const { return tapAnimation; }
    bool getZoneViewSortByName() const { return zoneViewSortByName; }
    bool getZoneViewSortByType() const { return zoneViewSortByType; }
    bool getSoundEnabled() const { return soundEnabled; }
    QString getSoundPath() const { return soundPath; }
    bool getPriceTagFeature() const { return priceTagFeature; }
    bool getIgnoreUnregisteredUsers() const { return ignoreUnregisteredUsers; }
    QString getPicUrl() const { return picUrl; }
    QString getPicUrlHq() const { return picUrlHq; }
    void copyPath(const QString &src, const QString &dst);
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
    void setPicDownload(int _picDownload);
    void setPicDownloadHq(int _picDownloadHq);
    void setNotificationsEnabled(int _notificationsEnabled);
    void setDoubleClickToPlay(int _doubleClickToPlay);
    void setPlayToStack(int _playToStack);
    void setCardInfoMinimized(int _cardInfoMinimized);
    void setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes);
    void setDisplayCardNames(int _displayCardNames);
    void setHorizontalHand(int _horizontalHand);
    void setInvertVerticalCoordinate(int _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setTapAnimation(int _tapAnimation);
    void setZoneViewSortByName(int _zoneViewSortByName);
    void setZoneViewSortByType(int _zoneViewSortByType);
    void setSoundEnabled(int _soundEnabled);
    void setSoundPath(const QString &_soundPath);
    void setPriceTagFeature(int _priceTagFeature);
    void setIgnoreUnregisteredUsers(bool _ignoreUnregisteredUsers);
    void setPicUrl(const QString &_picUrl);
    void setPicUrlHq(const QString &_picUrlHq);
};

extern SettingsCache *settingsCache;

#endif
