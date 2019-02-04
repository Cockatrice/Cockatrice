#include "settingscache.h"
#include "releasechannel.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <utility>

QString SettingsCache::getDataPath()
{
    if (isPortableBuild)
        return qApp->applicationDirPath() + "/data";
    else
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString SettingsCache::getSettingsPath()
{
    return getDataPath() + "/settings/";
}

void SettingsCache::translateLegacySettings()
{
    if (isPortableBuild)
        return;

    // Layouts
    QFile layoutFile(getSettingsPath() + "layouts/deckLayout.ini");
    if (layoutFile.exists())
        if (layoutFile.copy(getSettingsPath() + "layouts.ini"))
            layoutFile.remove();

    QStringList usedKeys;
    QSettings legacySetting;

    // Sets
    legacySetting.beginGroup("sets");
    QStringList setsGroups = legacySetting.childGroups();
    for (int i = 0; i < setsGroups.size(); i++) {
        legacySetting.beginGroup(setsGroups.at(i));
        cardDatabase().setEnabled(setsGroups.at(i), legacySetting.value("enabled").toBool());
        cardDatabase().setIsKnown(setsGroups.at(i), legacySetting.value("isknown").toBool());
        cardDatabase().setSortKey(setsGroups.at(i), legacySetting.value("sortkey").toUInt());
        legacySetting.endGroup();
    }
    QStringList setsKeys = legacySetting.allKeys();
    for (int i = 0; i < setsKeys.size(); ++i) {
        usedKeys.append("sets/" + setsKeys.at(i));
    }
    legacySetting.endGroup();

    // Servers
    legacySetting.beginGroup("server");
    servers().setPreviousHostLogin(legacySetting.value("previoushostlogin").toInt());
    servers().setPreviousHostList(legacySetting.value("previoushosts").toStringList());
    servers().setHostName(legacySetting.value("hostname").toString());
    servers().setPort(legacySetting.value("port").toString());
    servers().setPlayerName(legacySetting.value("playername").toString());
    servers().setPassword(legacySetting.value("password").toString());
    servers().setSavePassword(legacySetting.value("save_password").toInt());
    servers().setAutoConnect(legacySetting.value("auto_connect").toInt());
    servers().setFPHostName(legacySetting.value("fphostname").toString());
    servers().setFPPort(legacySetting.value("fpport").toString());
    servers().setFPPlayerName(legacySetting.value("fpplayername").toString());
    usedKeys.append(legacySetting.allKeys());
    QStringList allKeysServer = legacySetting.allKeys();
    for (int i = 0; i < allKeysServer.size(); ++i) {
        usedKeys.append("server/" + allKeysServer.at(i));
    }
    legacySetting.endGroup();

    // Messages
    legacySetting.beginGroup("messages");
    QStringList allMessages = legacySetting.allKeys();
    for (int i = 0; i < allMessages.size(); ++i) {
        if (allMessages.at(i) != "count") {
            QString temp = allMessages.at(i);
            int index = temp.remove("msg").toInt();
            messages().setMessageAt(index, legacySetting.value(allMessages.at(i)).toString());
        }
    }
    messages().setCount(legacySetting.value("count").toInt());
    QStringList allKeysmessages = legacySetting.allKeys();
    for (int i = 0; i < allKeysmessages.size(); ++i) {
        usedKeys.append("messages/" + allKeysmessages.at(i));
    }
    legacySetting.endGroup();

    // Game filters
    legacySetting.beginGroup("filter_games");
    gameFilters().setUnavailableGamesVisible(legacySetting.value("unavailable_games_visible").toBool());
    gameFilters().setShowPasswordProtectedGames(legacySetting.value("show_password_protected_games").toBool());
    gameFilters().setGameNameFilter(legacySetting.value("game_name_filter").toString());
    gameFilters().setShowBuddiesOnlyGames(legacySetting.value("show_buddies_only_games").toBool());
    gameFilters().setMinPlayers(legacySetting.value("min_players").toInt());

    if (legacySetting.value("max_players").toInt() > 1)
        gameFilters().setMaxPlayers(legacySetting.value("max_players").toInt());
    else
        gameFilters().setMaxPlayers(99); // This prevents a bug where no games will show if max was not set before

    QStringList allFilters = legacySetting.allKeys();
    for (int i = 0; i < allFilters.size(); ++i) {
        if (allFilters.at(i).startsWith("game_type")) {
            gameFilters().setGameHashedTypeEnabled(allFilters.at(i), legacySetting.value(allFilters.at(i)).toBool());
        }
    }
    QStringList allKeysfilter_games = legacySetting.allKeys();
    for (int i = 0; i < allKeysfilter_games.size(); ++i) {
        usedKeys.append("filter_games/" + allKeysfilter_games.at(i));
    }
    legacySetting.endGroup();

    QStringList allLegacyKeys = legacySetting.allKeys();
    for (int i = 0; i < allLegacyKeys.size(); ++i) {
        if (usedKeys.contains(allLegacyKeys.at(i)))
            continue;
        settings->setValue(allLegacyKeys.at(i), legacySetting.value(allLegacyKeys.at(i)));
    }
}

QString SettingsCache::getSafeConfigPath(QString configEntry, QString defaultPath) const
{
    QString tmp = settings->value(configEntry).toString();
    // if the config settings is empty or refers to a not-existing folder,
    // ensure that the defaut path exists and return it
    if (!QDir(tmp).exists() || tmp.isEmpty()) {
        if (!QDir().mkpath(defaultPath))
            qDebug() << "[SettingsCache] Could not create folder:" << defaultPath;
        tmp = defaultPath;
    }
    return tmp;
}

QString SettingsCache::getSafeConfigFilePath(QString configEntry, QString defaultPath) const
{
    QString tmp = settings->value(configEntry).toString();
    // if the config settings is empty or refers to a not-existing file,
    // return the default Path
    if (!QFile::exists(tmp) || tmp.isEmpty())
        tmp = std::move(defaultPath);
    return tmp;
}

SettingsCache::SettingsCache()
{
    // first, figure out if we are running in portable mode
    isPortableBuild = QFile::exists(qApp->applicationDirPath() + "/portable.dat");
    if (isPortableBuild)
        qDebug() << "Portable mode enabled";

    // define a dummy context that will be used where needed
    QString dummy = QT_TRANSLATE_NOOP("i18n", "English");

    QString dataPath = getDataPath();
    QString settingsPath = getSettingsPath();
    settings = new QSettings(settingsPath + "global.ini", QSettings::IniFormat, this);
    shortcutsSettings = new ShortcutsSettings(settingsPath, this);
    cardDatabaseSettings = new CardDatabaseSettings(settingsPath, this);
    serversSettings = new ServersSettings(settingsPath, this);
    messageSettings = new MessageSettings(settingsPath, this);
    gameFiltersSettings = new GameFiltersSettings(settingsPath, this);
    layoutsSettings = new LayoutsSettings(settingsPath, this);
    downloadSettings = new DownloadSettings(settingsPath, this);

    if (!QFile(settingsPath + "global.ini").exists())
        translateLegacySettings();

    // updates - don't reorder them or their index in the settings won't match
    // append channels one by one, or msvc will add them in the wrong order.
    releaseChannels << new StableReleaseChannel();
    releaseChannels << new BetaReleaseChannel();

    mbDownloadSpoilers = settings->value("personal/downloadspoilers", false).toBool();

    notifyAboutUpdates = settings->value("personal/updatenotification", true).toBool();
    notifyAboutNewVersion = settings->value("personal/newversionnotification", true).toBool();
    updateReleaseChannel = settings->value("personal/updatereleasechannel", 0).toInt();

    lang = settings->value("personal/lang").toString();
    keepalive = settings->value("personal/keepalive", 5).toInt();

    // tip of the day settings
    showTipsOnStartup = settings->value("tipOfDay/showTips", true).toBool();
    for (const auto &tipNumber : settings->value("tipOfDay/seenTips").toList()) {
        seenTips.append(tipNumber.toInt());
    }

    deckPath = getSafeConfigPath("paths/decks", dataPath + "/decks/");
    replaysPath = getSafeConfigPath("paths/replays", dataPath + "/replays/");
    picsPath = getSafeConfigPath("paths/pics", dataPath + "/pics/");
    // this has never been exposed as an user-configurable setting
    if (picsPath.endsWith("/")) {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "CUSTOM/");
    } else {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "/CUSTOM/");
    }
    // this has never been exposed as an user-configurable setting
    customCardDatabasePath = getSafeConfigPath("paths/customsets", dataPath + "/customsets/");

    cardDatabasePath = getSafeConfigFilePath("paths/carddatabase", dataPath + "/cards.xml");
    tokenDatabasePath = getSafeConfigFilePath("paths/tokendatabase", dataPath + "/tokens.xml");
    spoilerDatabasePath = getSafeConfigFilePath("paths/spoilerdatabase", dataPath + "/spoiler.xml");

    themeName = settings->value("theme/name").toString();

    // we only want to reset the cache once, then its up to the user
    bool updateCache = settings->value("revert/pixmapCacheSize", false).toBool();
    if (!updateCache) {
        pixmapCacheSize = PIXMAPCACHE_SIZE_DEFAULT;
        settings->setValue("personal/pixmapCacheSize", pixmapCacheSize);
        settings->setValue("personal/picturedownloadhq", false);
        settings->setValue("revert/pixmapCacheSize", true);
    } else
        pixmapCacheSize = settings->value("personal/pixmapCacheSize", PIXMAPCACHE_SIZE_DEFAULT).toInt();
    // sanity check
    if (pixmapCacheSize < PIXMAPCACHE_SIZE_MIN || pixmapCacheSize > PIXMAPCACHE_SIZE_MAX)
        pixmapCacheSize = PIXMAPCACHE_SIZE_DEFAULT;

    picDownload = settings->value("personal/picturedownload", true).toBool();

    mainWindowGeometry = settings->value("interface/main_window_geometry").toByteArray();
    tokenDialogGeometry = settings->value("interface/token_dialog_geometry").toByteArray();
    notificationsEnabled = settings->value("interface/notificationsenabled", true).toBool();
    spectatorNotificationsEnabled = settings->value("interface/specnotificationsenabled", false).toBool();
    doubleClickToPlay = settings->value("interface/doubleclicktoplay", true).toBool();
    playToStack = settings->value("interface/playtostack", true).toBool();
    annotateTokens = settings->value("interface/annotatetokens", false).toBool();
    tabGameSplitterSizes = settings->value("interface/tabgame_splittersizes").toByteArray();
    displayCardNames = settings->value("cards/displaycardnames", true).toBool();
    horizontalHand = settings->value("hand/horizontal", true).toBool();
    invertVerticalCoordinate = settings->value("table/invert_vertical", false).toBool();
    minPlayersForMultiColumnLayout = settings->value("interface/min_players_multicolumn", 4).toInt();
    tapAnimation = settings->value("cards/tapanimation", true).toBool();
    chatMention = settings->value("chat/mention", true).toBool();
    chatMentionCompleter = settings->value("chat/mentioncompleter", true).toBool();
    chatMentionForeground = settings->value("chat/mentionforeground", true).toBool();
    chatHighlightForeground = settings->value("chat/highlightforeground", true).toBool();
    chatMentionColor = settings->value("chat/mentioncolor", "A6120D").toString();
    chatHighlightColor = settings->value("chat/highlightcolor", "A6120D").toString();

    zoneViewSortByName = settings->value("zoneview/sortbyname", true).toBool();
    zoneViewSortByType = settings->value("zoneview/sortbytype", true).toBool();
    zoneViewPileView = settings->value("zoneview/pileview", true).toBool();

    soundEnabled = settings->value("sound/enabled", false).toBool();
    soundThemeName = settings->value("sound/theme").toString();

    maxFontSize = settings->value("game/maxfontsize", DEFAULT_FONT_SIZE).toInt();

    ignoreUnregisteredUsers = settings->value("chat/ignore_unregistered", false).toBool();
    ignoreUnregisteredUserMessages = settings->value("chat/ignore_unregistered_messages", false).toBool();

    scaleCards = settings->value("cards/scaleCards", true).toBool();
    showMessagePopups = settings->value("chat/showmessagepopups", true).toBool();
    showMentionPopups = settings->value("chat/showmentionpopups", true).toBool();
    roomHistory = settings->value("chat/roomhistory", true).toBool();

    leftJustified = settings->value("interface/leftjustified", false).toBool();

    masterVolume = settings->value("sound/mastervolume", 100).toInt();

    cardInfoViewMode = settings->value("cards/cardinfoviewmode", 0).toInt();
    highlightWords = settings->value("personal/highlightWords", QString()).toString();
    gameDescription = settings->value("game/gamedescription", "").toString();
    maxPlayers = settings->value("game/maxplayers", 2).toInt();
    gameTypes = settings->value("game/gametypes", "").toString();
    onlyBuddies = settings->value("game/onlybuddies", false).toBool();
    onlyRegistered = settings->value("game/onlyregistered", true).toBool();
    spectatorsAllowed = settings->value("game/spectatorsallowed", true).toBool();
    spectatorsNeedPassword = settings->value("game/spectatorsneedpassword", false).toBool();
    spectatorsCanTalk = settings->value("game/spectatorscantalk", false).toBool();
    spectatorsCanSeeEverything = settings->value("game/spectatorscanseeeverything", false).toBool();
    rememberGameSettings = settings->value("game/remembergamesettings", true).toBool();
    clientID = settings->value("personal/clientid", "notset").toString();
    clientVersion = settings->value("personal/clientversion", "notset").toString();
    knownMissingFeatures = settings->value("interface/knownmissingfeatures", "").toString();
}

void SettingsCache::setKnownMissingFeatures(QString _knownMissingFeatures)
{
    knownMissingFeatures = std::move(_knownMissingFeatures);
    settings->setValue("interface/knownmissingfeatures", knownMissingFeatures);
}

void SettingsCache::setCardInfoViewMode(const int _viewMode)
{
    cardInfoViewMode = _viewMode;
    settings->setValue("cards/cardinfoviewmode", cardInfoViewMode);
}

void SettingsCache::setHighlightWords(const QString &_highlightWords)
{
    highlightWords = _highlightWords;
    settings->setValue("personal/highlightWords", highlightWords);
}

void SettingsCache::setMasterVolume(int _masterVolume)
{
    masterVolume = _masterVolume;
    settings->setValue("sound/mastervolume", masterVolume);
    emit masterVolumeChanged(masterVolume);
}

void SettingsCache::setLeftJustified(const int _leftJustified)
{
    leftJustified = (bool)_leftJustified;
    settings->setValue("interface/leftjustified", leftJustified);
    emit handJustificationChanged();
}

void SettingsCache::setCardScaling(const int _scaleCards)
{
    scaleCards = (bool)_scaleCards;
    settings->setValue("cards/scaleCards", scaleCards);
}

void SettingsCache::setShowMessagePopups(const int _showMessagePopups)
{
    showMessagePopups = (bool)_showMessagePopups;
    settings->setValue("chat/showmessagepopups", showMessagePopups);
}

void SettingsCache::setShowMentionPopups(const int _showMentionPopus)
{
    showMentionPopups = (bool)_showMentionPopus;
    settings->setValue("chat/showmentionpopups", showMentionPopups);
}

void SettingsCache::setRoomHistory(const int _roomHistory)
{
    roomHistory = (bool)_roomHistory;
    settings->setValue("chat/roomhistory", roomHistory);
}

void SettingsCache::setLang(const QString &_lang)
{
    lang = _lang;
    settings->setValue("personal/lang", lang);
    emit langChanged();
}

void SettingsCache::setShowTipsOnStartup(bool _showTipsOnStartup)
{
    showTipsOnStartup = _showTipsOnStartup;
    settings->setValue("tipOfDay/showTips", showTipsOnStartup);
}

void SettingsCache::setSeenTips(const QList<int> &_seenTips)
{
    seenTips = _seenTips;
    QList<QVariant> storedTipList;
    for (auto tipNumber : seenTips) {
        storedTipList.append(tipNumber);
    }
    settings->setValue("tipOfDay/seenTips", storedTipList);
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
    // get a new value for customPicsPath, currently derived from picsPath
    customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "CUSTOM/");
    emit picsPathChanged();
}

void SettingsCache::setCardDatabasePath(const QString &_cardDatabasePath)
{
    cardDatabasePath = _cardDatabasePath;
    settings->setValue("paths/carddatabase", cardDatabasePath);
    emit cardDatabasePathChanged();
}

void SettingsCache::setSpoilerDatabasePath(const QString &_spoilerDatabasePath)
{
    spoilerDatabasePath = _spoilerDatabasePath;
    settings->setValue("paths/spoilerdatabase", spoilerDatabasePath);
    emit cardDatabasePathChanged();
}

void SettingsCache::setTokenDatabasePath(const QString &_tokenDatabasePath)
{
    tokenDatabasePath = _tokenDatabasePath;
    settings->setValue("paths/tokendatabase", tokenDatabasePath);
    emit cardDatabasePathChanged();
}

void SettingsCache::setThemeName(const QString &_themeName)
{
    themeName = _themeName;
    settings->setValue("theme/name", themeName);
    emit themeChanged();
}

void SettingsCache::setPicDownload(int _picDownload)
{
    picDownload = static_cast<bool>(_picDownload);
    settings->setValue("personal/picturedownload", picDownload);
    emit picDownloadChanged();
}

void SettingsCache::setNotificationsEnabled(int _notificationsEnabled)
{
    notificationsEnabled = static_cast<bool>(_notificationsEnabled);
    settings->setValue("interface/notificationsenabled", notificationsEnabled);
}

void SettingsCache::setSpectatorNotificationsEnabled(int _spectatorNotificationsEnabled)
{
    spectatorNotificationsEnabled = static_cast<bool>(_spectatorNotificationsEnabled);
    settings->setValue("interface/specnotificationsenabled", spectatorNotificationsEnabled);
}

void SettingsCache::setDoubleClickToPlay(int _doubleClickToPlay)
{
    doubleClickToPlay = static_cast<bool>(_doubleClickToPlay);
    settings->setValue("interface/doubleclicktoplay", doubleClickToPlay);
}

void SettingsCache::setPlayToStack(int _playToStack)
{
    playToStack = static_cast<bool>(_playToStack);
    settings->setValue("interface/playtostack", playToStack);
}

void SettingsCache::setAnnotateTokens(int _annotateTokens)
{
    annotateTokens = static_cast<bool>(_annotateTokens);
    settings->setValue("interface/annotatetokens", annotateTokens);
}

void SettingsCache::setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes)
{
    tabGameSplitterSizes = _tabGameSplitterSizes;
    settings->setValue("interface/tabgame_splittersizes", tabGameSplitterSizes);
}

void SettingsCache::setDisplayCardNames(int _displayCardNames)
{
    displayCardNames = static_cast<bool>(_displayCardNames);
    settings->setValue("cards/displaycardnames", displayCardNames);
    emit displayCardNamesChanged();
}

void SettingsCache::setHorizontalHand(int _horizontalHand)
{
    horizontalHand = static_cast<bool>(_horizontalHand);
    settings->setValue("hand/horizontal", horizontalHand);
    emit horizontalHandChanged();
}

void SettingsCache::setInvertVerticalCoordinate(int _invertVerticalCoordinate)
{
    invertVerticalCoordinate = static_cast<bool>(_invertVerticalCoordinate);
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
    tapAnimation = static_cast<bool>(_tapAnimation);
    settings->setValue("cards/tapanimation", tapAnimation);
}

void SettingsCache::setChatMention(int _chatMention)
{
    chatMention = static_cast<bool>(_chatMention);
    settings->setValue("chat/mention", chatMention);
}

void SettingsCache::setChatMentionCompleter(const int _enableMentionCompleter)
{
    chatMentionCompleter = (bool)_enableMentionCompleter;
    settings->setValue("chat/mentioncompleter", chatMentionCompleter);
    emit chatMentionCompleterChanged();
}

void SettingsCache::setChatMentionForeground(int _chatMentionForeground)
{
    chatMentionForeground = static_cast<bool>(_chatMentionForeground);
    settings->setValue("chat/mentionforeground", chatMentionForeground);
}

void SettingsCache::setChatHighlightForeground(int _chatHighlightForeground)
{
    chatHighlightForeground = static_cast<bool>(_chatHighlightForeground);
    settings->setValue("chat/highlightforeground", chatHighlightForeground);
}

void SettingsCache::setChatMentionColor(const QString &_chatMentionColor)
{
    chatMentionColor = _chatMentionColor;
    settings->setValue("chat/mentioncolor", chatMentionColor);
}

void SettingsCache::setChatHighlightColor(const QString &_chatHighlightColor)
{
    chatHighlightColor = _chatHighlightColor;
    settings->setValue("chat/highlightcolor", chatHighlightColor);
}

void SettingsCache::setZoneViewSortByName(int _zoneViewSortByName)
{
    zoneViewSortByName = static_cast<bool>(_zoneViewSortByName);
    settings->setValue("zoneview/sortbyname", zoneViewSortByName);
}

void SettingsCache::setZoneViewSortByType(int _zoneViewSortByType)
{
    zoneViewSortByType = static_cast<bool>(_zoneViewSortByType);
    settings->setValue("zoneview/sortbytype", zoneViewSortByType);
}

void SettingsCache::setZoneViewPileView(int _zoneViewPileView)
{
    zoneViewPileView = static_cast<bool>(_zoneViewPileView);
    settings->setValue("zoneview/pileview", zoneViewPileView);
}

void SettingsCache::setSoundEnabled(int _soundEnabled)
{
    soundEnabled = static_cast<bool>(_soundEnabled);
    settings->setValue("sound/enabled", soundEnabled);
    emit soundEnabledChanged();
}

void SettingsCache::setSoundThemeName(const QString &_soundThemeName)
{
    soundThemeName = _soundThemeName;
    settings->setValue("sound/theme", soundThemeName);
    emit soundThemeChanged();
}

void SettingsCache::setIgnoreUnregisteredUsers(int _ignoreUnregisteredUsers)
{
    ignoreUnregisteredUsers = static_cast<bool>(_ignoreUnregisteredUsers);
    settings->setValue("chat/ignore_unregistered", ignoreUnregisteredUsers);
}

void SettingsCache::setIgnoreUnregisteredUserMessages(int _ignoreUnregisteredUserMessages)
{
    ignoreUnregisteredUserMessages = static_cast<bool>(_ignoreUnregisteredUserMessages);
    settings->setValue("chat/ignore_unregistered_messages", ignoreUnregisteredUserMessages);
}

void SettingsCache::setMainWindowGeometry(const QByteArray &_mainWindowGeometry)
{
    mainWindowGeometry = _mainWindowGeometry;
    settings->setValue("interface/main_window_geometry", mainWindowGeometry);
}

void SettingsCache::setTokenDialogGeometry(const QByteArray &_tokenDialogGeometry)
{
    tokenDialogGeometry = _tokenDialogGeometry;
    settings->setValue("interface/token_dialog_geometry", tokenDialogGeometry);
}

void SettingsCache::setPixmapCacheSize(const int _pixmapCacheSize)
{
    pixmapCacheSize = _pixmapCacheSize;
    settings->setValue("personal/pixmapCacheSize", pixmapCacheSize);
    emit pixmapCacheSizeChanged(pixmapCacheSize);
}

void SettingsCache::setClientID(QString _clientID)
{
    clientID = std::move(_clientID);
    settings->setValue("personal/clientid", clientID);
}

void SettingsCache::setClientVersion(QString _clientVersion)
{
    clientVersion = std::move(_clientVersion);
    settings->setValue("personal/clientversion", clientVersion);
}

QStringList SettingsCache::getCountries() const
{
    static QStringList countries = QStringList() << "ad"
                                                 << "ae"
                                                 << "af"
                                                 << "ag"
                                                 << "ai"
                                                 << "al"
                                                 << "am"
                                                 << "ao"
                                                 << "aq"
                                                 << "ar"
                                                 << "as"
                                                 << "at"
                                                 << "au"
                                                 << "aw"
                                                 << "ax"
                                                 << "az"
                                                 << "ba"
                                                 << "bb"
                                                 << "bd"
                                                 << "be"
                                                 << "bf"
                                                 << "bg"
                                                 << "bh"
                                                 << "bi"
                                                 << "bj"
                                                 << "bl"
                                                 << "bm"
                                                 << "bn"
                                                 << "bo"
                                                 << "bq"
                                                 << "br"
                                                 << "bs"
                                                 << "bt"
                                                 << "bv"
                                                 << "bw"
                                                 << "by"
                                                 << "bz"
                                                 << "ca"
                                                 << "cc"
                                                 << "cd"
                                                 << "cf"
                                                 << "cg"
                                                 << "ch"
                                                 << "ci"
                                                 << "ck"
                                                 << "cl"
                                                 << "cm"
                                                 << "cn"
                                                 << "co"
                                                 << "cr"
                                                 << "cu"
                                                 << "cv"
                                                 << "cw"
                                                 << "cx"
                                                 << "cy"
                                                 << "cz"
                                                 << "de"
                                                 << "dj"
                                                 << "dk"
                                                 << "dm"
                                                 << "do"
                                                 << "dz"
                                                 << "ec"
                                                 << "ee"
                                                 << "eg"
                                                 << "eh"
                                                 << "er"
                                                 << "es"
                                                 << "et"
                                                 << "fi"
                                                 << "fj"
                                                 << "fk"
                                                 << "fm"
                                                 << "fo"
                                                 << "fr"
                                                 << "ga"
                                                 << "gb"
                                                 << "gd"
                                                 << "ge"
                                                 << "gf"
                                                 << "gg"
                                                 << "gh"
                                                 << "gi"
                                                 << "gl"
                                                 << "gm"
                                                 << "gn"
                                                 << "gp"
                                                 << "gq"
                                                 << "gr"
                                                 << "gs"
                                                 << "gt"
                                                 << "gu"
                                                 << "gw"
                                                 << "gy"
                                                 << "hk"
                                                 << "hm"
                                                 << "hn"
                                                 << "hr"
                                                 << "ht"
                                                 << "hu"
                                                 << "id"
                                                 << "ie"
                                                 << "il"
                                                 << "im"
                                                 << "in"
                                                 << "io"
                                                 << "iq"
                                                 << "ir"
                                                 << "is"
                                                 << "it"
                                                 << "je"
                                                 << "jm"
                                                 << "jo"
                                                 << "jp"
                                                 << "ke"
                                                 << "kg"
                                                 << "kh"
                                                 << "ki"
                                                 << "km"
                                                 << "kn"
                                                 << "kp"
                                                 << "kr"
                                                 << "kw"
                                                 << "ky"
                                                 << "kz"
                                                 << "la"
                                                 << "lb"
                                                 << "lc"
                                                 << "li"
                                                 << "lk"
                                                 << "lr"
                                                 << "ls"
                                                 << "lt"
                                                 << "lu"
                                                 << "lv"
                                                 << "ly"
                                                 << "ma"
                                                 << "mc"
                                                 << "md"
                                                 << "me"
                                                 << "mf"
                                                 << "mg"
                                                 << "mh"
                                                 << "mk"
                                                 << "ml"
                                                 << "mm"
                                                 << "mn"
                                                 << "mo"
                                                 << "mp"
                                                 << "mq"
                                                 << "mr"
                                                 << "ms"
                                                 << "mt"
                                                 << "mu"
                                                 << "mv"
                                                 << "mw"
                                                 << "mx"
                                                 << "my"
                                                 << "mz"
                                                 << "na"
                                                 << "nc"
                                                 << "ne"
                                                 << "nf"
                                                 << "ng"
                                                 << "ni"
                                                 << "nl"
                                                 << "no"
                                                 << "np"
                                                 << "nr"
                                                 << "nu"
                                                 << "nz"
                                                 << "om"
                                                 << "pa"
                                                 << "pe"
                                                 << "pf"
                                                 << "pg"
                                                 << "ph"
                                                 << "pk"
                                                 << "pl"
                                                 << "pm"
                                                 << "pn"
                                                 << "pr"
                                                 << "ps"
                                                 << "pt"
                                                 << "pw"
                                                 << "py"
                                                 << "qa"
                                                 << "re"
                                                 << "ro"
                                                 << "rs"
                                                 << "ru"
                                                 << "rw"
                                                 << "sa"
                                                 << "sb"
                                                 << "sc"
                                                 << "sd"
                                                 << "se"
                                                 << "sg"
                                                 << "sh"
                                                 << "si"
                                                 << "sj"
                                                 << "sk"
                                                 << "sl"
                                                 << "sm"
                                                 << "sn"
                                                 << "so"
                                                 << "sr"
                                                 << "ss"
                                                 << "st"
                                                 << "sv"
                                                 << "sx"
                                                 << "sy"
                                                 << "sz"
                                                 << "tc"
                                                 << "td"
                                                 << "tf"
                                                 << "tg"
                                                 << "th"
                                                 << "tj"
                                                 << "tk"
                                                 << "tl"
                                                 << "tm"
                                                 << "tn"
                                                 << "to"
                                                 << "tr"
                                                 << "tt"
                                                 << "tv"
                                                 << "tw"
                                                 << "tz"
                                                 << "ua"
                                                 << "ug"
                                                 << "um"
                                                 << "us"
                                                 << "uy"
                                                 << "uz"
                                                 << "va"
                                                 << "vc"
                                                 << "ve"
                                                 << "vg"
                                                 << "vi"
                                                 << "vn"
                                                 << "vu"
                                                 << "wf"
                                                 << "ws"
                                                 << "ye"
                                                 << "yt"
                                                 << "za"
                                                 << "zm"
                                                 << "zw";

    return countries;
}

void SettingsCache::setGameDescription(const QString _gameDescription)
{
    gameDescription = _gameDescription;
    settings->setValue("game/gamedescription", gameDescription);
}

void SettingsCache::setMaxPlayers(const int _maxPlayers)
{
    maxPlayers = _maxPlayers;
    settings->setValue("game/maxplayers", maxPlayers);
}

void SettingsCache::setGameTypes(const QString _gameTypes)
{
    gameTypes = _gameTypes;
    settings->setValue("game/gametypes", gameTypes);
}

void SettingsCache::setOnlyBuddies(const bool _onlyBuddies)
{
    onlyBuddies = _onlyBuddies;
    settings->setValue("game/onlybuddies", onlyBuddies);
}

void SettingsCache::setOnlyRegistered(const bool _onlyRegistered)
{
    onlyRegistered = _onlyRegistered;
    settings->setValue("game/onlyregistered", onlyRegistered);
}

void SettingsCache::setSpectatorsAllowed(const bool _spectatorsAllowed)
{
    spectatorsAllowed = _spectatorsAllowed;
    settings->setValue("game/spectatorsallowed", spectatorsAllowed);
}

void SettingsCache::setSpectatorsNeedPassword(const bool _spectatorsNeedPassword)
{
    spectatorsNeedPassword = _spectatorsNeedPassword;
    settings->setValue("game/spectatorsneedpassword", spectatorsNeedPassword);
}

void SettingsCache::setSpectatorsCanTalk(const bool _spectatorsCanTalk)
{
    spectatorsCanTalk = _spectatorsCanTalk;
    settings->setValue("game/spectatorscantalk", spectatorsCanTalk);
}

void SettingsCache::setSpectatorsCanSeeEverything(const bool _spectatorsCanSeeEverything)
{
    spectatorsCanSeeEverything = _spectatorsCanSeeEverything;
    settings->setValue("game/spectatorscanseeeverything", spectatorsCanSeeEverything);
}

void SettingsCache::setRememberGameSettings(const bool _rememberGameSettings)
{
    rememberGameSettings = _rememberGameSettings;
    settings->setValue("game/remembergamesettings", rememberGameSettings);
}

void SettingsCache::setNotifyAboutUpdate(int _notifyaboutupdate)
{
    notifyAboutUpdates = static_cast<bool>(_notifyaboutupdate);
    settings->setValue("personal/updatenotification", notifyAboutUpdates);
}

void SettingsCache::setNotifyAboutNewVersion(int _notifyaboutnewversion)
{
    notifyAboutNewVersion = static_cast<bool>(_notifyaboutnewversion);
    settings->setValue("personal/newversionnotification", notifyAboutNewVersion);
}

void SettingsCache::setDownloadSpoilerStatus(bool _spoilerStatus)
{
    mbDownloadSpoilers = _spoilerStatus;
    settings->setValue("personal/downloadspoilers", mbDownloadSpoilers);
    emit downloadSpoilerStatusChanged();
}

void SettingsCache::setUpdateReleaseChannel(int _updateReleaseChannel)
{
    updateReleaseChannel = _updateReleaseChannel;
    settings->setValue("personal/updatereleasechannel", updateReleaseChannel);
}

void SettingsCache::setMaxFontSize(int _max)
{
    maxFontSize = _max;
    settings->setValue("game/maxfontsize", maxFontSize);
}
