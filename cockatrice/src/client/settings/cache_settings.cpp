#include "cache_settings.h"

#include "../network/update/client/release_channel.h"
#include "card_counter_settings.h"

#include <QAbstractListModel>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGlobalStatic>
#include <QSettings>
#include <QStandardPaths>
#include <libcockatrice/settings/card_override_settings.h>
#include <utility>

Q_GLOBAL_STATIC(SettingsCache, settingsCache)

SettingsCache &SettingsCache::instance()
{
    return *settingsCache; // returns a QT managed singleton reference
}

QString SettingsCache::getDataPath()
{
    if (isPortableBuild)
        return qApp->applicationDirPath() + "/data";
    else
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString SettingsCache::getSettingsPath()
{
    return getDataPath() + "/settings/";
}

QString SettingsCache::getCachePath() const
{
    if (isPortableBuild)
        return qApp->applicationDirPath() + "/cache";
    else
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QString SettingsCache::getNetworkCachePath() const
{
    return getCachePath() + "/downloaded/";
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
        cardDatabase()->setEnabled(setsGroups.at(i), legacySetting.value("enabled").toBool());
        cardDatabase()->setIsKnown(setsGroups.at(i), legacySetting.value("isknown").toBool());
        cardDatabase()->setSortKey(setsGroups.at(i), legacySetting.value("sortkey").toUInt());
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
    gameFilters().setGameNameFilter(legacySetting.value("game_name_filter").toString());
    gameFilters().setHideIgnoredUserGames(legacySetting.value("hide_ignored_user_games").toBool());
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
    if (tmp.isEmpty() || !QDir(tmp).exists()) {
        if (!QDir().mkpath(defaultPath))
            qCInfo(SettingsCacheLog) << "[SettingsCache] Could not create folder:" << defaultPath;
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
        qCInfo(SettingsCacheLog) << "Portable mode enabled";

    // define a dummy context that will be used where needed
    QString dummy = QT_TRANSLATE_NOOP("i18n", "English");

    QString settingsPath = getSettingsPath();
    settings = new QSettings(settingsPath + "global.ini", QSettings::IniFormat, this);
    shortcutsSettings = new ShortcutsSettings(settingsPath, this);
    cardDatabaseSettings = new CardDatabaseSettings(settingsPath, this);
    serversSettings = new ServersSettings(settingsPath, this);
    messageSettings = new MessageSettings(settingsPath, this);
    gameFiltersSettings = new GameFiltersSettings(settingsPath, this);
    layoutsSettings = new LayoutsSettings(settingsPath, this);
    downloadSettings = new DownloadSettings(settingsPath, this);
    recentsSettings = new RecentsSettings(settingsPath, this);
    cardOverrideSettings = new CardOverrideSettings(settingsPath, this);
    debugSettings = new DebugSettings(settingsPath, this);

    cardCounterSettings = new CardCounterSettings(settingsPath, this);

    if (!QFile(settingsPath + "global.ini").exists())
        translateLegacySettings();

    // updates - don't reorder them or their index in the settings won't match
    // append channels one by one, or msvc will add them in the wrong order.
    releaseChannels << new StableReleaseChannel();
    releaseChannels << new BetaReleaseChannel();

    mbDownloadSpoilers = settings->value("personal/downloadspoilers", false).toBool();

    checkUpdatesOnStartup = settings->value("personal/startupUpdateCheck", true).toBool();
    startupCardUpdateCheckPromptForUpdate =
        settings->value("personal/startupCardUpdateCheckPromptForUpdate", true).toBool();
    startupCardUpdateCheckAlwaysUpdate = settings->value("personal/startupCardUpdateCheckAlwaysUpdate", false).toBool();
    cardUpdateCheckInterval = settings->value("personal/cardUpdateCheckInterval", 7).toInt();
    lastCardUpdateCheck = settings->value("personal/lastCardUpdateCheck", QDateTime::currentDateTime().date()).toDate();
    notifyAboutUpdates = settings->value("personal/updatenotification", true).toBool();
    notifyAboutNewVersion = settings->value("personal/newversionnotification", true).toBool();
    updateReleaseChannel = settings->value("personal/updatereleasechannel", 0).toInt();

    lang = settings->value("personal/lang").toString();
    keepalive = settings->value("personal/keepalive", 3).toInt();
    timeout = settings->value("personal/timeout", 5).toInt();

    // tip of the day settings
    showTipsOnStartup = settings->value("tipOfDay/showTips", true).toBool();
    for (const auto &tipNumber : settings->value("tipOfDay/seenTips").toList()) {
        seenTips.append(tipNumber.toInt());
    }

    loadPaths();

    themeName = settings->value("theme/name").toString();

    homeTabBackgroundSource = settings->value("home/background", "themed").toString();
    homeTabBackgroundShuffleFrequency = settings->value("home/background/shuffleTimer", 0).toInt();

    tabVisualDeckStorageOpen = settings->value("tabs/visualDeckStorage", true).toBool();
    tabServerOpen = settings->value("tabs/server", true).toBool();
    tabAccountOpen = settings->value("tabs/account", true).toBool();
    tabDeckStorageOpen = settings->value("tabs/deckStorage", true).toBool();
    tabReplaysOpen = settings->value("tabs/replays", true).toBool();
    tabAdminOpen = settings->value("tabs/admin", true).toBool();
    tabLogOpen = settings->value("tabs/log", true).toBool();

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

    networkCacheSize = settings->value("personal/networkCacheSize", NETWORK_CACHE_SIZE_DEFAULT).toInt();
    redirectCacheTtl = settings->value("personal/redirectCacheTtl", NETWORK_REDIRECT_CACHE_TTL_DEFAULT).toInt();

    picDownload = settings->value("personal/picturedownload", true).toBool();
    showStatusBar = settings->value("personal/showStatusBar", false).toBool();

    mainWindowGeometry = settings->value("interface/main_window_geometry").toByteArray();
    tokenDialogGeometry = settings->value("interface/token_dialog_geometry").toByteArray();
    setsDialogGeometry = settings->value("interface/sets_dialog_geometry").toByteArray();
    notificationsEnabled = settings->value("interface/notificationsenabled", true).toBool();
    spectatorNotificationsEnabled = settings->value("interface/specnotificationsenabled", false).toBool();
    buddyConnectNotificationsEnabled = settings->value("interface/buddyconnectnotificationsenabled", true).toBool();
    doubleClickToPlay = settings->value("interface/doubleclicktoplay", true).toBool();
    clickPlaysAllSelected = settings->value("interface/clickPlaysAllSelected", true).toBool();
    playToStack = settings->value("interface/playtostack", true).toBool();
    startingHandSize = settings->value("interface/startinghandsize", 7).toInt();
    annotateTokens = settings->value("interface/annotatetokens", false).toBool();
    tabGameSplitterSizes = settings->value("interface/tabgame_splittersizes").toByteArray();
    knownMissingFeatures = settings->value("interface/knownmissingfeatures", "").toString();
    useTearOffMenus = settings->value("interface/usetearoffmenus", true).toBool();
    cardViewInitialRowsMax = settings->value("interface/cardViewInitialRowsMax", 14).toInt();
    cardViewExpandedRowsMax = settings->value("interface/cardViewExpandedRowsMax", 20).toInt();
    closeEmptyCardView = settings->value("interface/closeEmptyCardView", true).toBool();
    focusCardViewSearchBar = settings->value("interface/focusCardViewSearchBar", true).toBool();

    showShortcuts = settings->value("menu/showshortcuts", true).toBool();
    displayCardNames = settings->value("cards/displaycardnames", true).toBool();
    roundCardCorners = settings->value("cards/roundcardcorners", true).toBool();
    overrideAllCardArtWithPersonalPreference =
        settings->value("cards/overrideallcardartwithpersonalpreference", false).toBool();
    bumpSetsWithCardsInDeckToTop = settings->value("cards/bumpsetswithcardsindecktotop", true).toBool();
    printingSelectorSortOrder = settings->value("cards/printingselectorsortorder", 1).toInt();
    printingSelectorCardSize = settings->value("cards/printingselectorcardsize", 100).toInt();
    includeRebalancedCards = settings->value("cards/includerebalancedcards", true).toBool();
    printingSelectorNavigationButtonsVisible =
        settings->value("cards/printingselectornavigationbuttonsvisible", true).toBool();
    deckEditorBannerCardComboBoxVisible =
        settings->value("interface/deckeditorbannercardcomboboxvisible", true).toBool();
    deckEditorTagsWidgetVisible = settings->value("interface/deckeditortagswidgetvisible", true).toBool();
    visualDeckStorageCardSize = settings->value("interface/visualdeckstoragecardsize", 100).toInt();
    visualDeckStorageSortingOrder = settings->value("interface/visualdeckstoragesortingorder", 0).toInt();
    visualDeckStorageShowFolders = settings->value("interface/visualdeckstorageshowfolders", true).toBool();
    visualDeckStorageShowTagFilter = settings->value("interface/visualdeckstorageshowtagfilter", true).toBool();
    visualDeckStorageDefaultTagsList =
        settings->value("interface/visualdeckstoragedefaulttagslist", defaultTags).toStringList();
    visualDeckStorageSearchFolderNames = settings->value("interface/visualdeckstoragesearchfoldernames", true).toBool();
    visualDeckStorageShowBannerCardComboBox =
        settings->value("interface/visualdeckstorageshowbannercardcombobox", true).toBool();
    visualDeckStorageShowTagsOnDeckPreviews =
        settings->value("interface/visualdeckstorageshowtagsondeckpreviews", true).toBool();
    visualDeckStorageDrawUnusedColorIdentities =
        settings->value("interface/visualdeckstoragedrawunusedcoloridentities", true).toBool();
    visualDeckStorageUnusedColorIdentitiesOpacity =
        settings->value("interface/visualdeckstorageunusedcoloridentitiesopacity", 15).toInt();
    visualDeckStorageTooltipType = settings->value("interface/visualdeckstoragetooltiptype", 0).toInt();
    visualDeckStoragePromptForConversion =
        settings->value("interface/visualdeckstoragepromptforconversion", true).toBool();
    visualDeckStorageAlwaysConvert = settings->value("interface/visualdeckstoragealwaysconvert", false).toBool();
    visualDeckStorageInGame = settings->value("interface/visualdeckstorageingame", true).toBool();
    visualDeckStorageSelectionAnimation =
        settings->value("interface/visualdeckstorageselectionanimation", true).toBool();
    defaultDeckEditorType = settings->value("interface/defaultDeckEditorType", 1).toInt();
    visualDatabaseDisplayFilterToMostRecentSetsEnabled =
        settings->value("interface/visualdatabasedisplayfiltertomostrecentsetsenabled", true).toBool();
    visualDatabaseDisplayFilterToMostRecentSetsAmount =
        settings->value("interface/visualdatabasedisplayfiltertomostrecentsetsamount", 10).toInt();
    visualDeckEditorSampleHandSize = settings->value("interface/visualdeckeditorsamplehandsize", 7).toInt();
    horizontalHand = settings->value("hand/horizontal", true).toBool();
    invertVerticalCoordinate = settings->value("table/invert_vertical", false).toBool();
    minPlayersForMultiColumnLayout = settings->value("interface/min_players_multicolumn", 4).toInt();
    tapAnimation = settings->value("cards/tapanimation", true).toBool();
    autoRotateSidewaysLayoutCards = settings->value("cards/autorotatesidewayslayoutcards", true).toBool();

    openDeckInNewTab = settings->value("editor/openDeckInNewTab", false).toBool();
    rewindBufferingMs = settings->value("replay/rewindBufferingMs", 200).toInt();
    chatMention = settings->value("chat/mention", true).toBool();
    chatMentionCompleter = settings->value("chat/mentioncompleter", true).toBool();
    chatMentionForeground = settings->value("chat/mentionforeground", true).toBool();
    chatHighlightForeground = settings->value("chat/highlightforeground", true).toBool();
    chatMentionColor = settings->value("chat/mentioncolor", "A6120D").toString();
    chatHighlightColor = settings->value("chat/highlightcolor", "A6120D").toString();

    zoneViewGroupByIndex = settings->value("zoneview/groupby", 1).toInt();
    zoneViewSortByIndex = settings->value("zoneview/sortby", 1).toInt();
    zoneViewPileView = settings->value("zoneview/pileview", true).toBool();

    soundEnabled = settings->value("sound/enabled", false).toBool();
    soundThemeName = settings->value("sound/theme").toString();

    maxFontSize = settings->value("game/maxfontsize", DEFAULT_FONT_SIZE).toInt();

    ignoreUnregisteredUsers = settings->value("chat/ignore_unregistered", false).toBool();
    ignoreUnregisteredUserMessages = settings->value("chat/ignore_unregistered_messages", false).toBool();

    scaleCards = settings->value("cards/scaleCards", true).toBool();
    verticalCardOverlapPercent = settings->value("cards/verticalCardOverlapPercent", 33).toInt();
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
    createGameAsSpectator = settings->value("game/creategameasspectator", false).toBool();
    defaultStartingLifeTotal = settings->value("game/defaultstartinglifetotal", 20).toInt();
    shareDecklistsOnLoad = settings->value("game/sharedecklistsonload", false).toBool();
    rememberGameSettings = settings->value("game/remembergamesettings", true).toBool();
    clientID = settings->value("personal/clientid", CLIENT_INFO_NOT_SET).toString();
    clientVersion = settings->value("personal/clientversion", CLIENT_INFO_NOT_SET).toString();
}

void SettingsCache::setUseTearOffMenus(bool _useTearOffMenus)
{
    useTearOffMenus = _useTearOffMenus;
    settings->setValue("interface/usetearoffmenus", useTearOffMenus);
    emit useTearOffMenusChanged(useTearOffMenus);
}

void SettingsCache::setCardViewInitialRowsMax(int _cardViewInitialRowsMax)
{
    cardViewInitialRowsMax = _cardViewInitialRowsMax;
    settings->setValue("interface/cardViewInitialRowsMax", cardViewInitialRowsMax);
}

void SettingsCache::setCardViewExpandedRowsMax(int value)
{
    cardViewExpandedRowsMax = value;
    settings->setValue("interface/cardViewExpandedRowsMax", cardViewExpandedRowsMax);
}

void SettingsCache::setCloseEmptyCardView(QT_STATE_CHANGED_T value)
{
    closeEmptyCardView = value;
    settings->setValue("interface/closeEmptyCardView", closeEmptyCardView);
}

void SettingsCache::setFocusCardViewSearchBar(QT_STATE_CHANGED_T value)
{
    focusCardViewSearchBar = value;
    settings->setValue("interface/focusCardViewSearchBar", focusCardViewSearchBar);
}

void SettingsCache::setKnownMissingFeatures(const QString &_knownMissingFeatures)
{
    knownMissingFeatures = _knownMissingFeatures;
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

void SettingsCache::setLeftJustified(const QT_STATE_CHANGED_T _leftJustified)
{
    leftJustified = (bool)_leftJustified;
    settings->setValue("interface/leftjustified", leftJustified);
    emit handJustificationChanged();
}

void SettingsCache::setCardScaling(const QT_STATE_CHANGED_T _scaleCards)
{
    scaleCards = (bool)_scaleCards;
    settings->setValue("cards/scaleCards", scaleCards);
}

void SettingsCache::setStackCardOverlapPercent(const int _verticalCardOverlapPercent)
{
    verticalCardOverlapPercent = _verticalCardOverlapPercent;
    settings->setValue("cards/verticalCardOverlapPercent", verticalCardOverlapPercent);
}

void SettingsCache::setShowMessagePopups(const QT_STATE_CHANGED_T _showMessagePopups)
{
    showMessagePopups = (bool)_showMessagePopups;
    settings->setValue("chat/showmessagepopups", showMessagePopups);
}

void SettingsCache::setShowMentionPopups(const QT_STATE_CHANGED_T _showMentionPopus)
{
    showMentionPopups = (bool)_showMentionPopus;
    settings->setValue("chat/showmentionpopups", showMentionPopups);
}

void SettingsCache::setRoomHistory(const QT_STATE_CHANGED_T _roomHistory)
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

void SettingsCache::setFiltersPath(const QString &_filtersPath)
{
    filtersPath = _filtersPath;
    settings->setValue("paths/filters", filtersPath);
}

void SettingsCache::setReplaysPath(const QString &_replaysPath)
{
    replaysPath = _replaysPath;
    settings->setValue("paths/replays", replaysPath);
}

void SettingsCache::setThemesPath(const QString &_themesPath)
{
    themesPath = _themesPath;
    settings->setValue("paths/themes", themesPath);
    emit themeChanged();
}

void SettingsCache::setCustomCardDatabasePath(const QString &_customCardDatabasePath)
{
    customCardDatabasePath = _customCardDatabasePath;
    settings->setValue("paths/customsets", customCardDatabasePath);
    emit cardDatabasePathChanged();
}

void SettingsCache::setPicsPath(const QString &_picsPath)
{
    picsPath = _picsPath;
    settings->setValue("paths/pics", picsPath);
    // get a new value for customPicsPath, currently derived from picsPath
    if (picsPath.endsWith("/")) {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "CUSTOM/");
    } else {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "/CUSTOM/");
    }
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

void SettingsCache::setHomeTabBackgroundSource(const QString &_backgroundSource)
{
    homeTabBackgroundSource = _backgroundSource;
    settings->setValue("home/background", homeTabBackgroundSource);
    emit homeTabBackgroundSourceChanged();
}

void SettingsCache::setHomeTabBackgroundShuffleFrequency(int _frequency)
{
    homeTabBackgroundShuffleFrequency = _frequency;
    settings->setValue("home/background/shuffleTimer", homeTabBackgroundShuffleFrequency);
    emit homeTabBackgroundShuffleFrequencyChanged();
}

void SettingsCache::setTabVisualDeckStorageOpen(bool value)
{
    tabVisualDeckStorageOpen = value;
    settings->setValue("tabs/visualDeckStorage", tabVisualDeckStorageOpen);
}

void SettingsCache::setTabServerOpen(bool value)
{
    tabServerOpen = value;
    settings->setValue("tabs/server", tabServerOpen);
}

void SettingsCache::setTabAccountOpen(bool value)
{
    tabAccountOpen = value;
    settings->setValue("tabs/account", tabAccountOpen);
}

void SettingsCache::setTabDeckStorageOpen(bool value)
{
    tabDeckStorageOpen = value;
    settings->setValue("tabs/deckStorage", tabDeckStorageOpen);
}

void SettingsCache::setTabReplaysOpen(bool value)
{
    tabReplaysOpen = value;
    settings->setValue("tabs/replays", tabReplaysOpen);
}

void SettingsCache::setTabAdminOpen(bool value)
{
    tabAdminOpen = value;
    settings->setValue("tabs/admin", tabAdminOpen);
}

void SettingsCache::setTabLogOpen(bool value)
{
    tabLogOpen = value;
    settings->setValue("tabs/log", tabLogOpen);
}

void SettingsCache::setPicDownload(QT_STATE_CHANGED_T _picDownload)
{
    picDownload = static_cast<bool>(_picDownload);
    settings->setValue("personal/picturedownload", picDownload);
    emit picDownloadChanged();
}

void SettingsCache::setShowStatusBar(bool value)
{
    showStatusBar = value;
    settings->setValue("personal/showStatusBar", showStatusBar);
    emit showStatusBarChanged(value);
}

void SettingsCache::setNotificationsEnabled(QT_STATE_CHANGED_T _notificationsEnabled)
{
    notificationsEnabled = static_cast<bool>(_notificationsEnabled);
    settings->setValue("interface/notificationsenabled", notificationsEnabled);
}

void SettingsCache::setSpectatorNotificationsEnabled(QT_STATE_CHANGED_T _spectatorNotificationsEnabled)
{
    spectatorNotificationsEnabled = static_cast<bool>(_spectatorNotificationsEnabled);
    settings->setValue("interface/specnotificationsenabled", spectatorNotificationsEnabled);
}

void SettingsCache::setBuddyConnectNotificationsEnabled(QT_STATE_CHANGED_T _buddyConnectNotificationsEnabled)
{
    buddyConnectNotificationsEnabled = static_cast<bool>(_buddyConnectNotificationsEnabled);
    settings->setValue("interface/buddyconnectnotificationsenabled", buddyConnectNotificationsEnabled);
}

void SettingsCache::setDoubleClickToPlay(QT_STATE_CHANGED_T _doubleClickToPlay)
{
    doubleClickToPlay = static_cast<bool>(_doubleClickToPlay);
    settings->setValue("interface/doubleclicktoplay", doubleClickToPlay);
}

void SettingsCache::setClickPlaysAllSelected(QT_STATE_CHANGED_T _clickPlaysAllSelected)
{
    clickPlaysAllSelected = static_cast<bool>(_clickPlaysAllSelected);
    settings->setValue("interface/clickPlaysAllSelected", clickPlaysAllSelected);
}

void SettingsCache::setPlayToStack(QT_STATE_CHANGED_T _playToStack)
{
    playToStack = static_cast<bool>(_playToStack);
    settings->setValue("interface/playtostack", playToStack);
}

void SettingsCache::setStartingHandSize(int _startingHandSize)
{
    startingHandSize = _startingHandSize;
    settings->setValue("interface/startinghandsize", startingHandSize);
}

void SettingsCache::setAnnotateTokens(QT_STATE_CHANGED_T _annotateTokens)
{
    annotateTokens = static_cast<bool>(_annotateTokens);
    settings->setValue("interface/annotatetokens", annotateTokens);
}

void SettingsCache::setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes)
{
    tabGameSplitterSizes = _tabGameSplitterSizes;
    settings->setValue("interface/tabgame_splittersizes", tabGameSplitterSizes);
}

void SettingsCache::setShowShortcuts(QT_STATE_CHANGED_T _showShortcuts)
{
    showShortcuts = static_cast<bool>(_showShortcuts);
    settings->setValue("menu/showshortcuts", showShortcuts);
}

void SettingsCache::setDisplayCardNames(QT_STATE_CHANGED_T _displayCardNames)
{
    displayCardNames = static_cast<bool>(_displayCardNames);
    settings->setValue("cards/displaycardnames", displayCardNames);
    emit displayCardNamesChanged();
}

void SettingsCache::setOverrideAllCardArtWithPersonalPreference(QT_STATE_CHANGED_T _overrideAllCardArt)
{
    overrideAllCardArtWithPersonalPreference = static_cast<bool>(_overrideAllCardArt);
    settings->setValue("cards/overrideallcardartwithpersonalpreference", overrideAllCardArtWithPersonalPreference);
    emit overrideAllCardArtWithPersonalPreferenceChanged(overrideAllCardArtWithPersonalPreference);
}

void SettingsCache::setBumpSetsWithCardsInDeckToTop(QT_STATE_CHANGED_T _bumpSetsWithCardsInDeckToTop)
{
    bumpSetsWithCardsInDeckToTop = static_cast<bool>(_bumpSetsWithCardsInDeckToTop);
    settings->setValue("cards/bumpsetswithcardsindecktotop", bumpSetsWithCardsInDeckToTop);
    emit bumpSetsWithCardsInDeckToTopChanged();
}

void SettingsCache::setPrintingSelectorSortOrder(int _printingSelectorSortOrder)
{
    printingSelectorSortOrder = _printingSelectorSortOrder;
    settings->setValue("cards/printingselectorsortorder", printingSelectorSortOrder);
    emit printingSelectorSortOrderChanged();
}

void SettingsCache::setPrintingSelectorCardSize(int _printingSelectorCardSize)
{
    printingSelectorCardSize = _printingSelectorCardSize;
    settings->setValue("cards/printingselectorcardsize", printingSelectorCardSize);
    emit printingSelectorCardSizeChanged();
}

void SettingsCache::setIncludeRebalancedCards(bool _includeRebalancedCards)
{
    if (includeRebalancedCards == _includeRebalancedCards)
        return;

    includeRebalancedCards = _includeRebalancedCards;
    settings->setValue("cards/includerebalancedcards", includeRebalancedCards);
    emit includeRebalancedCardsChanged(includeRebalancedCards);
}

void SettingsCache::setPrintingSelectorNavigationButtonsVisible(QT_STATE_CHANGED_T _navigationButtonsVisible)
{
    printingSelectorNavigationButtonsVisible = _navigationButtonsVisible;
    settings->setValue("cards/printingselectornavigationbuttonsvisible", printingSelectorNavigationButtonsVisible);
    emit printingSelectorNavigationButtonsVisibleChanged();
}

void SettingsCache::setDeckEditorBannerCardComboBoxVisible(QT_STATE_CHANGED_T _deckEditorBannerCardComboBoxVisible)
{
    deckEditorBannerCardComboBoxVisible = _deckEditorBannerCardComboBoxVisible;
    settings->setValue("interface/deckeditorbannercardcomboboxvisible", deckEditorBannerCardComboBoxVisible);
    emit deckEditorBannerCardComboBoxVisibleChanged(deckEditorBannerCardComboBoxVisible);
}

void SettingsCache::setDeckEditorTagsWidgetVisible(QT_STATE_CHANGED_T _deckEditorTagsWidgetVisible)
{
    deckEditorTagsWidgetVisible = _deckEditorTagsWidgetVisible;
    settings->setValue("interface/deckeditortagswidgetvisible", deckEditorTagsWidgetVisible);
    emit deckEditorTagsWidgetVisibleChanged(deckEditorTagsWidgetVisible);
}

void SettingsCache::setVisualDeckStorageSortingOrder(int _visualDeckStorageSortingOrder)
{
    visualDeckStorageSortingOrder = _visualDeckStorageSortingOrder;
    settings->setValue("interface/visualdeckstoragesortingorder", visualDeckStorageSortingOrder);
}

void SettingsCache::setVisualDeckStorageShowFolders(QT_STATE_CHANGED_T value)
{
    visualDeckStorageShowFolders = value;
    settings->setValue("interface/visualdeckstorageshowfolders", visualDeckStorageShowFolders);
}

void SettingsCache::setVisualDeckStorageShowTagFilter(QT_STATE_CHANGED_T _showTags)
{
    visualDeckStorageShowTagFilter = _showTags;
    settings->setValue("interface/visualdeckstorageshowtagfilter", visualDeckStorageShowTagFilter);
    emit visualDeckStorageShowTagFilterChanged(visualDeckStorageShowTagFilter);
}

void SettingsCache::setVisualDeckStorageDefaultTagsList(QStringList _defaultTagsList)
{
    visualDeckStorageDefaultTagsList = _defaultTagsList;
    settings->setValue("interface/visualdeckstoragedefaulttagslist", visualDeckStorageDefaultTagsList);
    emit visualDeckStorageDefaultTagsListChanged();
}

void SettingsCache::setVisualDeckStorageSearchFolderNames(QT_STATE_CHANGED_T value)
{
    visualDeckStorageSearchFolderNames = value;
    settings->setValue("interface/visualdeckstoragesearchfoldernames", visualDeckStorageSearchFolderNames);
}

void SettingsCache::setVisualDeckStorageShowBannerCardComboBox(QT_STATE_CHANGED_T _showBannerCardComboBox)
{
    visualDeckStorageShowBannerCardComboBox = _showBannerCardComboBox;
    settings->setValue("interface/visualdeckstorageshowbannercardcombobox", visualDeckStorageShowBannerCardComboBox);
    emit visualDeckStorageShowBannerCardComboBoxChanged(visualDeckStorageShowBannerCardComboBox);
}

void SettingsCache::setVisualDeckStorageShowTagsOnDeckPreviews(QT_STATE_CHANGED_T _showTags)
{
    visualDeckStorageShowTagsOnDeckPreviews = _showTags;
    settings->setValue("interface/visualdeckstorageshowtagsondeckpreviews", visualDeckStorageShowTagsOnDeckPreviews);
    emit visualDeckStorageShowTagsOnDeckPreviewsChanged(visualDeckStorageShowTagsOnDeckPreviews);
}

void SettingsCache::setVisualDeckStorageCardSize(int _visualDeckStorageCardSize)
{
    visualDeckStorageCardSize = _visualDeckStorageCardSize;
    settings->setValue("interface/visualdeckstoragecardsize", visualDeckStorageCardSize);
    emit visualDeckStorageCardSizeChanged();
}

void SettingsCache::setVisualDeckStorageDrawUnusedColorIdentities(
    QT_STATE_CHANGED_T _visualDeckStorageDrawUnusedColorIdentities)
{
    visualDeckStorageDrawUnusedColorIdentities = _visualDeckStorageDrawUnusedColorIdentities;
    settings->setValue("interface/visualdeckstoragedrawunusedcoloridentities",
                       visualDeckStorageDrawUnusedColorIdentities);
    emit visualDeckStorageDrawUnusedColorIdentitiesChanged(visualDeckStorageDrawUnusedColorIdentities);
}

void SettingsCache::setVisualDeckStorageUnusedColorIdentitiesOpacity(int _visualDeckStorageUnusedColorIdentitiesOpacity)
{
    visualDeckStorageUnusedColorIdentitiesOpacity = _visualDeckStorageUnusedColorIdentitiesOpacity;
    settings->setValue("interface/visualdeckstorageunusedcoloridentitiesopacity",
                       visualDeckStorageUnusedColorIdentitiesOpacity);
    emit visualDeckStorageUnusedColorIdentitiesOpacityChanged(visualDeckStorageUnusedColorIdentitiesOpacity);
}

void SettingsCache::setVisualDeckStorageTooltipType(int value)
{
    visualDeckStorageTooltipType = value;
    settings->setValue("interface/visualdeckstoragetooltiptype", visualDeckStorageTooltipType);
}

void SettingsCache::setVisualDeckStoragePromptForConversion(bool _visualDeckStoragePromptForConversion)
{
    visualDeckStoragePromptForConversion = _visualDeckStoragePromptForConversion;
    settings->setValue("interface/visualdeckstoragepromptforconversion", visualDeckStoragePromptForConversion);
}

void SettingsCache::setVisualDeckStorageAlwaysConvert(bool _visualDeckStorageAlwaysConvert)
{
    visualDeckStorageAlwaysConvert = _visualDeckStorageAlwaysConvert;
    settings->setValue("interface/visualdeckstoragealwaysconvert", visualDeckStorageAlwaysConvert);
}

void SettingsCache::setVisualDeckStorageInGame(QT_STATE_CHANGED_T value)
{
    visualDeckStorageInGame = value;
    settings->setValue("interface/visualdeckstorageingame", visualDeckStorageInGame);
    emit visualDeckStorageInGameChanged(visualDeckStorageInGame);
}

void SettingsCache::setVisualDeckStorageSelectionAnimation(QT_STATE_CHANGED_T value)
{
    visualDeckStorageSelectionAnimation = value;
    settings->setValue("interface/visualdeckstorageselectionanimation", visualDeckStorageSelectionAnimation);
    emit visualDeckStorageSelectionAnimationChanged(visualDeckStorageSelectionAnimation);
}

void SettingsCache::setDefaultDeckEditorType(int value)
{
    defaultDeckEditorType = value;
    settings->setValue("interface/defaultDeckEditorType", defaultDeckEditorType);
}

void SettingsCache::setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(QT_STATE_CHANGED_T _enabled)
{
    visualDatabaseDisplayFilterToMostRecentSetsEnabled = _enabled;
    settings->setValue("interface/visualdatabasedisplayfiltertomostrecentsetsenabled",
                       visualDatabaseDisplayFilterToMostRecentSetsEnabled);
    emit visualDatabaseDisplayFilterToMostRecentSetsEnabledChanged(visualDatabaseDisplayFilterToMostRecentSetsEnabled);
}

void SettingsCache::setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int _amount)
{
    visualDatabaseDisplayFilterToMostRecentSetsAmount = _amount;
    settings->setValue("interface/visualdatabasedisplayfiltertomostrecentsetsamount",
                       visualDatabaseDisplayFilterToMostRecentSetsAmount);
    emit visualDatabaseDisplayFilterToMostRecentSetsAmountChanged(visualDatabaseDisplayFilterToMostRecentSetsAmount);
}

void SettingsCache::setVisualDeckEditorSampleHandSize(int _amount)
{
    visualDeckEditorSampleHandSize = _amount;
    settings->setValue("interface/visualdeckeditorsamplehandsize", visualDeckEditorSampleHandSize);
    emit visualDeckEditorSampleHandSizeAmountChanged(visualDeckEditorSampleHandSize);
}

void SettingsCache::setHorizontalHand(QT_STATE_CHANGED_T _horizontalHand)
{
    horizontalHand = static_cast<bool>(_horizontalHand);
    settings->setValue("hand/horizontal", horizontalHand);
    emit horizontalHandChanged();
}

void SettingsCache::setInvertVerticalCoordinate(QT_STATE_CHANGED_T _invertVerticalCoordinate)
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

void SettingsCache::setTapAnimation(QT_STATE_CHANGED_T _tapAnimation)
{
    tapAnimation = static_cast<bool>(_tapAnimation);
    settings->setValue("cards/tapanimation", tapAnimation);
}

void SettingsCache::setAutoRotateSidewaysLayoutCards(QT_STATE_CHANGED_T _autoRotateSidewaysLayoutCards)
{
    autoRotateSidewaysLayoutCards = static_cast<bool>(_autoRotateSidewaysLayoutCards);
    settings->setValue("cards/autorotatesidewayslayoutcards", autoRotateSidewaysLayoutCards);
}

void SettingsCache::setOpenDeckInNewTab(QT_STATE_CHANGED_T _openDeckInNewTab)
{
    openDeckInNewTab = static_cast<bool>(_openDeckInNewTab);
    settings->setValue("editor/openDeckInNewTab", openDeckInNewTab);
}

void SettingsCache::setRewindBufferingMs(int _rewindBufferingMs)
{
    rewindBufferingMs = _rewindBufferingMs;
    settings->setValue("replay/rewindBufferingMs", rewindBufferingMs);
}

void SettingsCache::setChatMention(QT_STATE_CHANGED_T _chatMention)
{
    chatMention = static_cast<bool>(_chatMention);
    settings->setValue("chat/mention", chatMention);
}

void SettingsCache::setChatMentionCompleter(const QT_STATE_CHANGED_T _enableMentionCompleter)
{
    chatMentionCompleter = (bool)_enableMentionCompleter;
    settings->setValue("chat/mentioncompleter", chatMentionCompleter);
    emit chatMentionCompleterChanged();
}

void SettingsCache::setChatMentionForeground(QT_STATE_CHANGED_T _chatMentionForeground)
{
    chatMentionForeground = static_cast<bool>(_chatMentionForeground);
    settings->setValue("chat/mentionforeground", chatMentionForeground);
}

void SettingsCache::setChatHighlightForeground(QT_STATE_CHANGED_T _chatHighlightForeground)
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

void SettingsCache::setZoneViewGroupByIndex(int _zoneViewGroupByIndex)
{
    zoneViewGroupByIndex = _zoneViewGroupByIndex;
    settings->setValue("zoneview/groupby", zoneViewGroupByIndex);
}

void SettingsCache::setZoneViewSortByIndex(int _zoneViewSortByIndex)
{
    zoneViewSortByIndex = _zoneViewSortByIndex;
    settings->setValue("zoneview/sortby", zoneViewSortByIndex);
}

void SettingsCache::setZoneViewPileView(QT_STATE_CHANGED_T _zoneViewPileView)
{
    zoneViewPileView = static_cast<bool>(_zoneViewPileView);
    settings->setValue("zoneview/pileview", zoneViewPileView);
}

void SettingsCache::setSoundEnabled(QT_STATE_CHANGED_T _soundEnabled)
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

void SettingsCache::setIgnoreUnregisteredUsers(QT_STATE_CHANGED_T _ignoreUnregisteredUsers)
{
    ignoreUnregisteredUsers = static_cast<bool>(_ignoreUnregisteredUsers);
    settings->setValue("chat/ignore_unregistered", ignoreUnregisteredUsers);
}

void SettingsCache::setIgnoreUnregisteredUserMessages(QT_STATE_CHANGED_T _ignoreUnregisteredUserMessages)
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

void SettingsCache::setSetsDialogGeometry(const QByteArray &_setsDialogGeometry)
{
    setsDialogGeometry = _setsDialogGeometry;
    settings->setValue("interface/sets_dialog_geometry", setsDialogGeometry);
}

void SettingsCache::setPixmapCacheSize(const int _pixmapCacheSize)
{
    pixmapCacheSize = _pixmapCacheSize;
    settings->setValue("personal/pixmapCacheSize", pixmapCacheSize);
    emit pixmapCacheSizeChanged(pixmapCacheSize);
}

void SettingsCache::setNetworkCacheSizeInMB(const int _networkCacheSize)
{
    networkCacheSize = _networkCacheSize;
    settings->setValue("personal/networkCacheSize", networkCacheSize);
    emit networkCacheSizeChanged(networkCacheSize);
}

void SettingsCache::setNetworkRedirectCacheTtl(const int _redirectCacheTtl)
{
    redirectCacheTtl = _redirectCacheTtl;
    settings->setValue("personal/redirectCacheSize", redirectCacheTtl);
    emit redirectCacheTtlChanged(redirectCacheTtl);
}

void SettingsCache::setClientID(const QString &_clientID)
{
    clientID = _clientID;
    settings->setValue("personal/clientid", clientID);
}

void SettingsCache::setClientVersion(const QString &_clientVersion)
{
    clientVersion = _clientVersion;
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
                                                 << "eu"
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
                                                 << "xk"
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

void SettingsCache::setCreateGameAsSpectator(const bool _createGameAsSpectator)
{
    createGameAsSpectator = _createGameAsSpectator;
    settings->setValue("game/creategameasspectator", createGameAsSpectator);
}

void SettingsCache::setDefaultStartingLifeTotal(const int _defaultStartingLifeTotal)
{
    defaultStartingLifeTotal = _defaultStartingLifeTotal;
    settings->setValue("game/defaultstartinglifetotal", defaultStartingLifeTotal);
}

void SettingsCache::setShareDecklistsOnLoad(const bool _shareDecklistsOnLoad)
{
    shareDecklistsOnLoad = _shareDecklistsOnLoad;
    settings->setValue("game/sharedecklistsonload", shareDecklistsOnLoad);
}

void SettingsCache::setCheckUpdatesOnStartup(QT_STATE_CHANGED_T value)
{
    checkUpdatesOnStartup = static_cast<bool>(value);
    settings->setValue("personal/startupUpdateCheck", checkUpdatesOnStartup);
}

void SettingsCache::setStartupCardUpdateCheckPromptForUpdate(bool value)
{
    startupCardUpdateCheckPromptForUpdate = value;
    settings->setValue("personal/startupCardUpdateCheckPromptForUpdate", startupCardUpdateCheckPromptForUpdate);
}

void SettingsCache::setStartupCardUpdateCheckAlwaysUpdate(bool value)
{
    startupCardUpdateCheckAlwaysUpdate = value;
    settings->setValue("personal/startupCardUpdateCheckAlwaysUpdate", startupCardUpdateCheckAlwaysUpdate);
}

void SettingsCache::setCardUpdateCheckInterval(int value)
{
    cardUpdateCheckInterval = value;
    settings->setValue("personal/cardUpdateCheckInterval", cardUpdateCheckInterval);
}

void SettingsCache::setLastCardUpdateCheck(QDate value)
{
    lastCardUpdateCheck = value;
    settings->setValue("personal/lastCardUpdateCheck", lastCardUpdateCheck);
}

void SettingsCache::setRememberGameSettings(const bool _rememberGameSettings)
{
    rememberGameSettings = _rememberGameSettings;
    settings->setValue("game/remembergamesettings", rememberGameSettings);
}

void SettingsCache::setNotifyAboutUpdate(QT_STATE_CHANGED_T _notifyaboutupdate)
{
    notifyAboutUpdates = static_cast<bool>(_notifyaboutupdate);
    settings->setValue("personal/updatenotification", notifyAboutUpdates);
}

void SettingsCache::setNotifyAboutNewVersion(QT_STATE_CHANGED_T _notifyaboutnewversion)
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

void SettingsCache::setUpdateReleaseChannelIndex(int value)
{
    updateReleaseChannel = value;
    settings->setValue("personal/updatereleasechannel", updateReleaseChannel);
}

void SettingsCache::setMaxFontSize(int _max)
{
    maxFontSize = _max;
    settings->setValue("game/maxfontsize", maxFontSize);
}

void SettingsCache::setRoundCardCorners(bool _roundCardCorners)
{
    if (_roundCardCorners == roundCardCorners)
        return;

    roundCardCorners = _roundCardCorners;
    settings->setValue("cards/roundcardcorners", _roundCardCorners);
    emit roundCardCornersChanged(roundCardCorners);
}

void SettingsCache::loadPaths()
{
    QString dataPath = getDataPath();
    deckPath = getSafeConfigPath("paths/decks", dataPath + "/decks/");
    filtersPath = getSafeConfigPath("paths/filters", dataPath + "/filters/");
    replaysPath = getSafeConfigPath("paths/replays", dataPath + "/replays/");
    themesPath = getSafeConfigPath("paths/themes", dataPath + "/themes/");
    picsPath = getSafeConfigPath("paths/pics", dataPath + "/pics/");
    redirectCachePath = getSafeConfigPath("paths/redirects", getCachePath() + "/redirects/");
    // this has never been exposed as an user-configurable setting
    if (picsPath.endsWith("/")) {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "CUSTOM/");
    } else {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "/CUSTOM/");
    }
    customCardDatabasePath = getSafeConfigPath("paths/customsets", dataPath + "/customsets/");

    cardDatabasePath = getSafeConfigFilePath("paths/carddatabase", dataPath + "/cards.xml");
    tokenDatabasePath = getSafeConfigFilePath("paths/tokendatabase", dataPath + "/tokens.xml");
    spoilerDatabasePath = getSafeConfigFilePath("paths/spoilerdatabase", dataPath + "/spoiler.xml");
}

void SettingsCache::resetPaths()
{
    QStringList databasePaths{customCardDatabasePath, cardDatabasePath, spoilerDatabasePath, tokenDatabasePath};
    QString picsPath_ = picsPath;
    settings->remove("paths"); // removes all keys in paths/*
    loadPaths();
    if (databasePaths !=
        QStringList{customCardDatabasePath, cardDatabasePath, spoilerDatabasePath, tokenDatabasePath}) {
        emit cardDatabasePathChanged();
    }
    if (picsPath_ != picsPath) {
        emit picsPathChanged();
    }
}

CardCounterSettings &SettingsCache::cardCounters() const
{
    return *cardCounterSettings;
}
