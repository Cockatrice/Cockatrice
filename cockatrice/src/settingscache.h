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
	void picDownloadChanged();
private:
	QSettings *settings;
	
	QString lang;
	QString deckPath, picsPath, cardDatabasePath;
	bool picDownload;
	bool doubleClickToPlay;
	bool economicGrid;
public:
	SettingsCache();
	QString getLang() const { return lang; }
	QString getDeckPath() const { return deckPath; }
	QString getPicsPath() const { return picsPath; }
	QString getCardDatabasePath() const { return cardDatabasePath; }
	bool getPicDownload() const { return picDownload; }
	bool getDoubleClickToPlay() const { return doubleClickToPlay; }
	bool getEconomicGrid() const { return economicGrid; }
public slots:
	void setLang(const QString &_lang);
	void setDeckPath(const QString &_deckPath);
	void setPicsPath(const QString &_picsPath);
	void setCardDatabasePath(const QString &_cardDatabasePath);
	void setPicDownload(int _picDownload);
	void setDoubleClickToPlay(int _doubleClickToPlay);
	void setEconomicGrid(int _economicGrid);
};

extern SettingsCache *settingsCache;

#endif