#include "settingscache.h"
#include <QSettings>
#include <QDir>

SettingsCache::SettingsCache()
{
    settings = new QSettings(this);

    lang = settings->value("personal/lang").toString();

    deckPath = settings->value("paths/decks").toString();
    replaysPath = settings->value("paths/replays").toString();
    picsPath = settings->value("paths/pics").toString();
    cardDatabasePath = settings->value("paths/carddatabase").toString();
    tokenDatabasePath = settings->value("paths/tokendatabase").toString();

    handBgPath = settings->value("zonebg/hand").toString();
    stackBgPath = settings->value("zonebg/stack").toString();
    tableBgPath = settings->value("zonebg/table").toString();
    playerBgPath = settings->value("zonebg/playerarea").toString();
    cardBackPicturePath = settings->value("paths/cardbackpicture").toString();

    picDownload = settings->value("personal/picturedownload", true).toBool();
    picDownloadHq = settings->value("personal/picturedownloadhq", false).toBool();
    picUrl = settings->value("personal/picUrl", PIC_URL_DEFAULT).toString();
    picUrlHq = settings->value("personal/picUrlHq", PIC_URL_HQ_DEFAULT).toString();

    mainWindowGeometry = settings->value("interface/main_window_geometry").toByteArray();
    notificationsEnabled = settings->value("interface/notificationsenabled", true).toBool();
    doubleClickToPlay = settings->value("interface/doubleclicktoplay", true).toBool();
    playToStack = settings->value("interface/playtostack", false).toBool();
    cardInfoMinimized = settings->value("interface/cardinfominimized", 0).toInt();
    tabGameSplitterSizes = settings->value("interface/tabgame_splittersizes").toByteArray();
    displayCardNames = settings->value("cards/displaycardnames", true).toBool();
    horizontalHand = settings->value("hand/horizontal", true).toBool();
    invertVerticalCoordinate = settings->value("table/invert_vertical", false).toBool();
    minPlayersForMultiColumnLayout = settings->value("interface/min_players_multicolumn", 5).toInt();
    tapAnimation = settings->value("cards/tapanimation", true).toBool();

    zoneViewSortByName = settings->value("zoneview/sortbyname", true).toBool();
    zoneViewSortByType = settings->value("zoneview/sortbytype", true).toBool();

    soundEnabled = settings->value("sound/enabled", false).toBool();
    soundPath = settings->value("sound/path").toString();

    priceTagFeature = settings->value("deckeditor/pricetags", false).toBool();

    ignoreUnregisteredUsers = settings->value("chat/ignore_unregistered", false).toBool();
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

void SettingsCache::setReplaysPath(const QString &_replaysPath)
{
    replaysPath = _replaysPath;
    settings->setValue("paths/replays", replaysPath);
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

void SettingsCache::setTokenDatabasePath(const QString &_tokenDatabasePath)
{
    tokenDatabasePath = _tokenDatabasePath;
    settings->setValue("paths/tokendatabase", tokenDatabasePath);
    emit tokenDatabasePathChanged();
}

void SettingsCache::setHandBgPath(const QString &_handBgPath)
{
    handBgPath = _handBgPath;
    settings->setValue("zonebg/hand", handBgPath);
    emit handBgPathChanged();
}

void SettingsCache::setStackBgPath(const QString &_stackBgPath)
{
    stackBgPath = _stackBgPath;
    settings->setValue("zonebg/stack", stackBgPath);
    emit stackBgPathChanged();
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
    settings->setValue("zonebg/playerarea", playerBgPath);
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

void SettingsCache::setPicDownloadHq(int _picDownloadHq)
{
    picDownloadHq = _picDownloadHq;
    settings->setValue("personal/picturedownloadhq", picDownloadHq);
    emit picDownloadHqChanged();
}

void SettingsCache::setPicUrl(const QString &_picUrl)
{
    picUrl = _picUrl;
    settings->setValue("personal/picUrl", picUrl);
}

void SettingsCache::setPicUrlHq(const QString &_picUrlHq)
{
    picUrlHq = _picUrlHq;
    settings->setValue("personal/picUrlHq", picUrlHq);
}

void SettingsCache::setNotificationsEnabled(int _notificationsEnabled)
{
    notificationsEnabled = _notificationsEnabled;
    settings->setValue("interface/notificationsenabled", notificationsEnabled);
}

void SettingsCache::setDoubleClickToPlay(int _doubleClickToPlay)
{
    doubleClickToPlay = _doubleClickToPlay;
    settings->setValue("interface/doubleclicktoplay", doubleClickToPlay);
}

void SettingsCache::setPlayToStack(int _playToStack)
{
    playToStack = _playToStack;
    settings->setValue("interface/playtostack", playToStack);
}

void SettingsCache::setCardInfoMinimized(int _cardInfoMinimized)
{
        cardInfoMinimized = _cardInfoMinimized;
    settings->setValue("interface/cardinfominimized", cardInfoMinimized);
}

void SettingsCache::setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes)
{
    tabGameSplitterSizes = _tabGameSplitterSizes;
    settings->setValue("interface/tabgame_splittersizes", tabGameSplitterSizes);
}

void SettingsCache::setDisplayCardNames(int _displayCardNames)
{
    displayCardNames = _displayCardNames;
    settings->setValue("cards/displaycardnames", displayCardNames);
    emit displayCardNamesChanged();
}

void SettingsCache::setHorizontalHand(int _horizontalHand)
{
    horizontalHand = _horizontalHand;
    settings->setValue("hand/horizontal", horizontalHand);
    emit horizontalHandChanged();
}

void SettingsCache::setInvertVerticalCoordinate(int _invertVerticalCoordinate)
{
    invertVerticalCoordinate = _invertVerticalCoordinate;
    settings->setValue("table/invert_vertical", invertVerticalCoordinate);
    emit invertVerticalCoordinateChanged();
}

void SettingsCache::setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout)
{
    minPlayersForMultiColumnLayout = _minPlayersForMultiColumnLayout;
    settings->setValue("interface/min_players_multicolumn", minPlayersForMultiColumnLayout);
    emit minPlayersForMultiColumnLayoutChanged();
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

void SettingsCache::setSoundEnabled(int _soundEnabled)
{
    soundEnabled = _soundEnabled;
    settings->setValue("sound/enabled", soundEnabled);
    emit soundEnabledChanged();
}

void SettingsCache::setSoundPath(const QString &_soundPath)
{
    soundPath = _soundPath;
    settings->setValue("sound/path", soundPath);
    emit soundPathChanged();
}

void SettingsCache::setPriceTagFeature(int _priceTagFeature)
{
    priceTagFeature = _priceTagFeature;
    settings->setValue("deckeditor/pricetags", priceTagFeature);
}

void SettingsCache::setIgnoreUnregisteredUsers(bool _ignoreUnregisteredUsers)
{
    ignoreUnregisteredUsers = _ignoreUnregisteredUsers;
    settings->setValue("chat/ignore_unregistered", ignoreUnregisteredUsers);
    emit ignoreUnregisteredUsersChanged();
}

void SettingsCache::setMainWindowGeometry(const QByteArray &_mainWindowGeometry)
{
    mainWindowGeometry = _mainWindowGeometry;
    settings->setValue("interface/main_window_geometry", mainWindowGeometry);
}

void SettingsCache::copyPath(const QString &src, const QString &dst)
{
    // test source && return if inexistent
    QDir dir(src);
    if (! dir.exists())
        return;
    // test destination && create if inexistent
    QDir tmpDir(dst);
    if (!tmpDir.exists())
    {
        tmpDir.setPath(QDir::rootPath());
        tmpDir.mkdir(dst);
    }

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyPath(src+ QDir::separator() + d, dst_path);
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
}
