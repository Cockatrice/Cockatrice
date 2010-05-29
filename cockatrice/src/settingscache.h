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
	void picDownloadChanged();
	void horizontalHandChanged();
	void economicGridChanged();
private:
	QSettings *settings;
	
	QString lang;
	QString deckPath, picsPath, cardDatabasePath;
	QString handBgPath, tableBgPath, playerBgPath;
	bool picDownload;
	bool doubleClickToPlay;
	bool horizontalHand;
	bool economicGrid;
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
	bool getPicDownload() const { return picDownload; }
	bool getDoubleClickToPlay() const { return doubleClickToPlay; }
	bool getHorizontalHand() const { return horizontalHand; }
	bool getEconomicGrid() const { return economicGrid; }
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
	void setPicDownload(int _picDownload);
	void setDoubleClickToPlay(int _doubleClickToPlay);
	void setHorizontalHand(int _horizontalHand);
	void setEconomicGrid(int _economicGrid);
	void setZoneViewSortByName(int _zoneViewSortByName);
	void setZoneViewSortByType(int _zoneViewSortByType);
};

extern SettingsCache *settingsCache;

#endif
