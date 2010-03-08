#include "settingscache.h"
#include <QSettings>

SettingsCache::SettingsCache()
{
	settings = new QSettings;
	
	lang = settings->value("personal/lang").toString();
	
	deckPath = settings->value("paths/decks").toString();
	picsPath = settings->value("paths/pics").toString();
	cardDatabasePath = settings->value("paths/carddatabase").toString();
	
	handBgPath = settings->value("zonebg/hand").toString();
	tableBgPath = settings->value("zonebg/table").toString();
	playerBgPath = settings->value("zonebg/playerarea").toString();
	
	picDownload = settings->value("personal/picturedownload", 0).toInt();
	doubleClickToPlay = settings->value("interface/doubleclicktoplay", 1).toInt();
	economicGrid = settings->value("table/economic", 0).toInt();
}

void SettingsCache::setLang(const QString &_lang)
{
	lang = _lang;
	settings->setValue("personal/lang", lang);
	emit langChanged();
}

void SettingsCache::setDeckPath(const QString &_deckPath)
{
	deckPath = _deckPath;
	settings->setValue("paths/decks", deckPath);
}

void SettingsCache::setPicsPath(const QString &_picsPath)
{
	picsPath = _picsPath;
	settings->setValue("paths/pics", picsPath);
	emit picsPathChanged();
}

void SettingsCache::setCardDatabasePath(const QString &_cardDatabasePath)
{
	cardDatabasePath = _cardDatabasePath;
	settings->setValue("paths/carddatabase", cardDatabasePath);
	emit cardDatabasePathChanged();
}

void SettingsCache::setHandBgPath(const QString &_handBgPath)
{
	handBgPath = _handBgPath;
	settings->setValue("zonebg/hand", handBgPath);
	emit handBgPathChanged();
}

void SettingsCache::setTableBgPath(const QString &_tableBgPath)
{
	tableBgPath = _tableBgPath;
	settings->setValue("zonebg/table", tableBgPath);
	emit tableBgPathChanged();
}

void SettingsCache::setPlayerBgPath(const QString &_playerBgPath)
{
	playerBgPath = _playerBgPath;
	settings->setValue("zonebg/player", playerBgPath);
	emit playerBgPathChanged();
}

void SettingsCache::setPicDownload(int _picDownload)
{
	picDownload = _picDownload;
	settings->setValue("personal/picturedownload", picDownload);
	emit picDownloadChanged();
}

void SettingsCache::setDoubleClickToPlay(int _doubleClickToPlay)
{
	doubleClickToPlay = _doubleClickToPlay;
	settings->setValue("interface/doubleclicktoplay", doubleClickToPlay);
}

void SettingsCache::setEconomicGrid(int _economicGrid)
{
	economicGrid = _economicGrid;
	settings->setValue("table/economic", economicGrid);
	emit economicGridChanged();
}

void SettingsCache::setZoneViewSortByName(int _zoneViewSortByName)
{
	zoneViewSortByName = _zoneViewSortByName;
	settings->setValue("zoneview/sortbyname", zoneViewSortByName);
}

void SettingsCache::setZoneViewSortByType(int _zoneViewSortByType)
{
	zoneViewSortByType = _zoneViewSortByType;
	settings->setValue("zoneview/sortbytype", zoneViewSortByType);
}
