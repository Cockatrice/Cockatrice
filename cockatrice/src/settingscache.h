#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include <QObject>

class QSettings;

class SettingsCache : public QObject {
	Q_OBJECT
signals:
	void langChanged();
	void picsPathChanged();
	void cardDatabasePathChanged();
	void handBgPathChanged();
	void stackBgPathChanged();
	void tableBgPathChanged();
	void playerBgPathChanged();
	void cardBackPicturePathChanged();
	void picDownloadChanged();
	void displayCardNamesChanged();
	void horizontalHandChanged();
	void invertVerticalCoordinateChanged();
	void minPlayersForMultiColumnLayoutChanged();
	void soundPathChanged();
private:
	QSettings *settings;
	
	QString lang;
	QString deckPath, replaysPath, picsPath, cardDatabasePath;
	QString handBgPath, stackBgPath, tableBgPath, playerBgPath, cardBackPicturePath;
	bool picDownload;
	bool doubleClickToPlay;
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
public:
	SettingsCache();
	QString getLang() const { return lang; }
	QString getDeckPath() const { return deckPath; }
	QString getReplaysPath() const { return replaysPath; }
	QString getPicsPath() const { return picsPath; }
	QString getCardDatabasePath() const { return cardDatabasePath; }
	QString getHandBgPath() const { return handBgPath; }
	QString getStackBgPath() const { return stackBgPath; }
	QString getTableBgPath() const { return tableBgPath; }
	QString getPlayerBgPath() const { return playerBgPath; }
	QString getCardBackPicturePath() const { return cardBackPicturePath; }
	bool getPicDownload() const { return picDownload; }
	bool getDoubleClickToPlay() const { return doubleClickToPlay; }
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
public slots:
	void setLang(const QString &_lang);
	void setDeckPath(const QString &_deckPath);
	void setReplaysPath(const QString &_replaysPath);
	void setPicsPath(const QString &_picsPath);
	void setCardDatabasePath(const QString &_cardDatabasePath);
	void setHandBgPath(const QString &_handBgPath);
	void setStackBgPath(const QString &_stackBgPath);
	void setTableBgPath(const QString &_tableBgPath);
	void setPlayerBgPath(const QString &_playerBgPath);
	void setCardBackPicturePath(const QString &_cardBackPicturePath);
	void setPicDownload(int _picDownload);
	void setDoubleClickToPlay(int _doubleClickToPlay);
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
};

extern SettingsCache *settingsCache;

#endif
