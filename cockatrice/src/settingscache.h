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
	void tableBgPathChanged();
	void playerBgPathChanged();
	void cardBackPicturePathChanged();
	void picDownloadChanged();
	void horizontalHandChanged();
	void economicalGridChanged();
	void invertVerticalCoordinateChanged();
private:
	QSettings *settings;
	
	QString lang;
	QString deckPath, picsPath, cardDatabasePath;
	QString handBgPath, tableBgPath, playerBgPath, cardBackPicturePath;
	bool picDownload;
	bool doubleClickToPlay;
	bool cardInfoMinimized;
	bool horizontalHand;
	bool economicalGrid;
	bool invertVerticalCoordinate;
	bool tapAnimation;
	bool zoneViewSortByName, zoneViewSortByType;
public:
	SettingsCache();
	QString getLang() const { return lang; }
	QString getDeckPath() const { return deckPath; }
	QString getPicsPath() const { return picsPath; }
	QString getCardDatabasePath() const { return cardDatabasePath; }
	QString getHandBgPath() const { return handBgPath; }
	QString getTableBgPath() const { return tableBgPath; }
	QString getPlayerBgPath() const { return playerBgPath; }
	QString getCardBackPicturePath() const { return cardBackPicturePath; }
	bool getPicDownload() const { return picDownload; }
	bool getDoubleClickToPlay() const { return doubleClickToPlay; }
	bool getCardInfoMinimized() const { return cardInfoMinimized; }
	bool getHorizontalHand() const { return horizontalHand; }
	bool getEconomicalGrid() const { return economicalGrid; }
	bool getInvertVerticalCoordinate() const { return invertVerticalCoordinate; }
	bool getTapAnimation() const { return tapAnimation; }
	bool getZoneViewSortByName() const { return zoneViewSortByName; }
	bool getZoneViewSortByType() const { return zoneViewSortByType; }
public slots:
	void setLang(const QString &_lang);
	void setDeckPath(const QString &_deckPath);
	void setPicsPath(const QString &_picsPath);
	void setCardDatabasePath(const QString &_cardDatabasePath);
	void setHandBgPath(const QString &_handBgPath);
	void setTableBgPath(const QString &_tableBgPath);
	void setPlayerBgPath(const QString &_playerBgPath);
	void setCardBackPicturePath(const QString &_cardBackPicturePath);
	void setPicDownload(int _picDownload);
	void setDoubleClickToPlay(int _doubleClickToPlay);
	void setCardInfoMinimized(bool _cardInfoMinimized);
	void setHorizontalHand(int _horizontalHand);
	void setEconomicalGrid(int _economicalGrid);
	void setInvertVerticalCoordinate(int _invertVerticalCoordinate);
	void setTapAnimation(int _tapAnimation);
	void setZoneViewSortByName(int _zoneViewSortByName);
	void setZoneViewSortByType(int _zoneViewSortByType);
};

extern SettingsCache *settingsCache;

#endif
