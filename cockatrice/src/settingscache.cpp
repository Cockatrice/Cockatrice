#include "settingscache.h"
#include <QSettings>

SettingsCache::SettingsCache()
{
	settings = new QSettings(this);
	
	lang = settings->value("personal/lang").toString();
	
	deckPath = settings->value("paths/decks").toString();
	picsPath = settings->value("paths/pics").toString();
	cardDatabasePath = settings->value("paths/carddatabase").toString();
	
	handBgPath = settings->value("zonebg/hand").toString();
	tableBgPath = settings->value("zonebg/table").toString();
	playerBgPath = settings->value("zonebg/playerarea").toString();
	cardBackPicturePath = settings->value("paths/cardbackpicture").toString();
	
	picDownload = settings->value("personal/picturedownload", false).toBool();
	doubleClickToPlay = settings->value("interface/doubleclicktoplay", true).toBool();
	cardInfoMinimized = settings->value("interface/cardinfominimized", false).toBool();
	horizontalHand = settings->value("hand/horizontal", false).toBool();
	economicalGrid = settings->value("table/economic", false).toBool();
	invertVerticalCoordinate = settings->value("table/invert_vertical", false).toBool();
	tapAnimation = settings->value("cards/tapanimation", true).toBool();
	
	zoneViewSortByName = settings->value("zoneview/sortbyname", false).toBool();
	zoneViewSortByType = settings->value("zoneview/sortbytype", false).toBool();
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

void SettingsCache::setCardBackPicturePath(const QString &_cardBackPicturePath)
{
	cardBackPicturePath = _cardBackPicturePath;
	settings->setValue("paths/cardbackpicture", cardBackPicturePath);
	emit cardBackPicturePathChanged();
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

void SettingsCache::setCardInfoMinimized(bool _cardInfoMinimized)
{
	cardInfoMinimized = _cardInfoMinimized;
	settings->setValue("interface/cardinfominimized", cardInfoMinimized);
}

void SettingsCache::setHorizontalHand(int _horizontalHand)
{
	horizontalHand = _horizontalHand;
	settings->setValue("hand/horizontal", horizontalHand);
	emit horizontalHandChanged();
}

void SettingsCache::setEconomicalGrid(int _economicalGrid)
{
	economicalGrid = _economicalGrid;
	settings->setValue("table/economic", economicalGrid);
	emit economicalGridChanged();
}

void SettingsCache::setInvertVerticalCoordinate(int _invertVerticalCoordinate)
{
	invertVerticalCoordinate = _invertVerticalCoordinate;
	settings->setValue("table/invert_vertical", invertVerticalCoordinate);
	emit invertVerticalCoordinateChanged();
}

void SettingsCache::setTapAnimation(int _tapAnimation)
{
	tapAnimation = _tapAnimation;
	settings->setValue("cards/tapanimation", tapAnimation);
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
