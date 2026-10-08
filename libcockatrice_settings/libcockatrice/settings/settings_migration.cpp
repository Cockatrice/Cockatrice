#include "settings_migration.h"

#include <QChar>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFlags>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QMessageLogger>
#include <QSettings>
#include <QSharedPointer>
#include <QStringList>
#include <QStringLiteral>
#include <QVariant>
#include <algorithm>
#include <utility>

inline Q_LOGGING_CATEGORY(SettingsMigrationLog, "settings_migration");

static const QString MIGRATION_SENTINEL_KEY = QStringLiteral("migration/perfile_complete");

/**
 * True when global.ini holds keys that migrateSettingsFromGlobalIni() would still move, i.e. the
 * exact condition that function uses to decide whether it has work to do. If a user runs an older
 * Cockatrice build that writes to global.ini (non-sentinel keys) and then upgrades back, those new
 * keys would otherwise be silently ignored, so they are re-migrated instead.
 */
static bool hasPendingGlobalIniSettings(const QString &settingsPath)
{
    if (!QFile::exists(settingsPath + "global.ini")) {
        return false;
    }

    QSettings globalIni(settingsPath + "global.ini", QSettings::IniFormat);
    if (!globalIni.value(MIGRATION_SENTINEL_KEY, false).toBool()) {
        return true;
    }

    // The sentinel is set, so any key besides it was written after the migration, i.e. by an
    // older build sharing this file. Those are exactly the keys to re-migrate.
    QStringList otherKeys = globalIni.allKeys();
    otherKeys.removeAll(MIGRATION_SENTINEL_KEY);
    return !otherKeys.isEmpty();
}

static void migrateTabsSettings(const QString &settingsPath, QSettings &globalIni)
{
    if (!globalIni.contains("tabs/visualDeckStorage") && !globalIni.contains("tabs/server") &&
        !globalIni.contains("tabs/account") && !globalIni.contains("tabs/deckStorage") &&
        !globalIni.contains("tabs/replays") && !globalIni.contains("tabs/admin") && !globalIni.contains("tabs/log")) {
        return;
    }

    QSettings tabsIni(settingsPath + "tabs.ini", QSettings::IniFormat);
    tabsIni.setValue("tabs/visualDeckStorage", globalIni.value("tabs/visualDeckStorage", true));
    tabsIni.setValue("tabs/server", globalIni.value("tabs/server", true));
    tabsIni.setValue("tabs/account", globalIni.value("tabs/account", true));
    tabsIni.setValue("tabs/deckStorage", globalIni.value("tabs/deckStorage", true));
    tabsIni.setValue("tabs/replays", globalIni.value("tabs/replays", true));
    tabsIni.setValue("tabs/admin", globalIni.value("tabs/admin", true));
    tabsIni.setValue("tabs/log", globalIni.value("tabs/log", true));
}

static void migrateSoundSettings(const QString &settingsPath, QSettings &globalIni)
{
    if (!globalIni.contains("sound/enabled") && !globalIni.contains("sound/theme") &&
        !globalIni.contains("sound/mastervolume")) {
        return;
    }

    QSettings soundIni(settingsPath + "sound.ini", QSettings::IniFormat);
    soundIni.setValue("sound/enabled", globalIni.value("sound/enabled", false));
    soundIni.setValue("sound/theme", globalIni.value("sound/theme"));
    soundIni.setValue("sound/masterVolume", globalIni.value("sound/mastervolume", 100));
}

static void migrateGameSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> gameKeyMap = {
        {"game/maxplayers", "game/maxPlayers"},
        {"game/gamedescription", "game/gameDescription"},
        {"game/gametypes", "game/gameTypes"},
        {"game/onlybuddies", "game/onlyBuddies"},
        {"game/onlyregistered", "game/onlyRegistered"},
        {"game/spectatorsallowed", "game/spectatorsAllowed"},
        {"game/spectatorsneedpassword", "game/spectatorsNeedPassword"},
        {"game/spectatorscantalk", "game/spectatorsCanTalk"},
        {"game/spectatorscanseeeverything", "game/spectatorsCanSeeEverything"},
        {"game/creategameasspectator", "game/createGameAsSpectator"},
        {"game/defaultstartinglifetotal", "game/defaultStartingLifeTotal"},
        {"game/sharedecklistsonload", "game/shareDecklistsOnLoad"},
        {"game/remembergamesettings", "game/rememberGameSettings"},
        {"localgameoptions/maxplayers", "localgameoptions/maxPlayers"},
        {"localgameoptions/startinglifetotal", "localgameoptions/startingLifeTotal"},
        {"localgameoptions/remembersettings", "localgameoptions/rememberSettings"},
    };

    bool hasAny = false;
    for (auto it = gameKeyMap.constBegin(); it != gameKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings gameIni(settingsPath + "game.ini", QSettings::IniFormat);
    for (auto it = gameKeyMap.constBegin(); it != gameKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            gameIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateChatSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> chatKeyMap = {
        {"chat/mention", "chat/mention"},
        {"chat/mentioncompleter", "chat/mentionCompleter"},
        {"chat/mentioncolor", "chat/mentionColor"},
        {"chat/highlightcolor", "chat/highlightColor"},
        {"chat/mentionforeground", "chat/mentionForeground"},
        {"chat/highlightforeground", "chat/highlightForeground"},
        {"chat/ignore_unregistered", "chat/ignoreUnregistered"},
        {"chat/ignore_unregistered_messages", "chat/ignoreUnregisteredMessages"},
        {"chat/ignore_nonbuddy_messages", "chat/ignoreNonBuddyMessages"},
        {"chat/showmessagepopups", "chat/showMessagePopups"},
        {"chat/showmentionpopups", "chat/showMentionPopups"},
        {"chat/roomhistory", "chat/roomHistory"},
        {"chat/highlightwords", "chat/highlightWords"},
        // Legacy highlight words lived under [personal], but the chat settings
        // class reads them from [chat]
        {"personal/highlightWords", "chat/highlightWords"},
    };
    bool hasAny = false;
    for (auto it = chatKeyMap.constBegin(); it != chatKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings chatIni(settingsPath + "chat.ini", QSettings::IniFormat);
    for (auto it = chatKeyMap.constBegin(); it != chatKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            chatIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateCacheStorageSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> cacheStorageKeyMap = {
        {"personal/pixmapCacheSize", "cache_storage/pixmapCacheSize"},
        {"personal/networkCacheSize", "cache_storage/networkCacheSize"},
        {"personal/redirectCacheTtl", "cache_storage/redirectCacheTtl"},
        {"personal/cardPictureLoaderCacheMethod", "cache_storage/cardPictureLoaderCacheMethod"},
        {"personal/localCardImageStorageNamingScheme", "cache_storage/localCardImageStorageNamingScheme"},
    };
    bool hasAny = false;
    for (auto it = cacheStorageKeyMap.constBegin(); it != cacheStorageKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings cacheStorageIni(settingsPath + "cache_storage.ini", QSettings::IniFormat);
    for (auto it = cacheStorageKeyMap.constBegin(); it != cacheStorageKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            cacheStorageIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateUpdatesSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> updateKeyMap = {
        {"personal/startupUpdateCheck", "updates/startupUpdateCheck"},
        {"personal/startupCardUpdateCheckPromptForUpdate", "updates/startupCardUpdateCheckPromptForUpdate"},
        {"personal/startupCardUpdateCheckAlwaysUpdate", "updates/startupCardUpdateCheckAlwaysUpdate"},
        {"personal/cardUpdateCheckInterval", "updates/cardUpdateCheckInterval"},
        {"personal/lastCardUpdateCheck", "updates/lastCardUpdateCheck"},
        {"personal/alwaysEnableNewSets", "updates/alwaysEnableNewSets"},
        {"personal/updatenotification", "updates/updateNotification"},
        {"personal/newversionnotification", "updates/newVersionNotification"},
        {"personal/updatereleasechannel", "updates/updateReleaseChannel"},
    };
    bool hasAny = false;
    for (auto it = updateKeyMap.constBegin(); it != updateKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings updatesIni(settingsPath + "updates.ini", QSettings::IniFormat);
    for (auto it = updateKeyMap.constBegin(); it != updateKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            updatesIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migratePersonalSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QStringList personalRootKeys = {"personal/lang"};
    const QStringList tipKeys = {"tipOfDay/showTips", "tipOfDay/seenTips"};

    bool hasAny = false;
    for (const auto &key : personalRootKeys) {
        if (globalIni.contains(key)) {
            hasAny = true;
        }
    }
    for (const auto &key : tipKeys) {
        if (globalIni.contains(key)) {
            hasAny = true;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings personalIni(settingsPath + "personal.ini", QSettings::IniFormat);
    for (const auto &key : personalRootKeys) {
        if (globalIni.contains(key)) {
            personalIni.setValue(key, globalIni.value(key));
        }
    }
    for (const auto &key : tipKeys) {
        if (globalIni.contains(key)) {
            personalIni.setValue(key, globalIni.value(key));
        }
    }
}

static void migrateCardsDisplaySettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> cardsKeyMap = {
        {"cards/displaycardnames", "cards/displayCardNames"},
        {"cards/roundcardcorners", "cards/roundCardCorners"},
        {"cards/overrideallcardartwithpersonalpreference", "cards/overrideAllCardArtWithPersonalPreference"},
        {"cards/bumpsetswithcardsindecktotop", "cards/bumpSetsWithCardsInDeckToTop"},
        {"cards/includerebalancedcards", "cards/includerebalancedcards"},
        {"cards/tapanimation", "cards/tapAnimation"},
        {"cards/autorotatesidewayslayoutcards", "cards/autoRotateSidewaysLayoutCards"},
        {"cards/scaleCards", "cards/scaleCards"},
        {"cards/verticalCardOverlapPercent", "cards/verticalCardOverlapPercent"},
        {"cards/cardinfoviewmode", "cards/cardInfoViewMode"},
        {"cards/printingselectorsortorder", "cards/printingSelector/sortOrder"},
        {"cards/printingselectornavigationbuttonsvisible", "cards/printingSelector/navigationButtonsVisible"},
        {"cards/printingselectorcardsize", "cards/cardSize/printingSelector"},
        {"interface/visualdeckstoragecardsize", "cards/cardSize/visualDeckStorage"},
        {"interface/visualdatabasedisplaycardsize", "cards/cardSize/visualDatabaseDisplay"},
        {"interface/visualdeckeditorcardsize", "cards/cardSize/visualDeckEditor"},
        {"interface/edhreccardsize", "cards/cardSize/edhrec"},
        {"interface/archidektpreviewsize", "cards/cardSize/archidektPreview"},
        {"interface/visualdeckeditorsamplehandsize", "cards/cardSize/sampleHandSize"},
    };

    bool hasAny = false;
    for (auto it = cardsKeyMap.constBegin(); it != cardsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings cardsIni(settingsPath + "cards_display.ini", QSettings::IniFormat);
    for (auto it = cardsKeyMap.constBegin(); it != cardsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            cardsIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateCardCounterSettings(const QString &settingsPath, QSettings &globalIni)
{
    QStringList counterKeys;
    const QStringList allKeys = globalIni.allKeys();
    for (const auto &key : allKeys) {
        if (key.startsWith("cards/counters/")) {
            counterKeys.append(key);
        }
    }
    if (counterKeys.isEmpty()) {
        return;
    }

    QSettings countersIni(settingsPath + "card_counters.ini", QSettings::IniFormat);
    for (const auto &key : counterKeys) {
        countersIni.setValue(key, globalIni.value(key));
    }
}

static void migrateInterfaceSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> interfaceKeyMap = {
        {"interface/usetearoffmenus", "interface/useTearOffMenus"},
        {"interface/cardViewInitialRowsMax", "interface/cardViewInitialRowsMax"},
        {"interface/cardViewExpandedRowsMax", "interface/cardViewExpandedRowsMax"},
        {"interface/closeEmptyCardView", "interface/closeEmptyCardView"},
        {"interface/focusCardViewSearchBar", "interface/focusCardViewSearchBar"},
        {"interface/keepGameChatFocus", "interface/keepGameChatFocus"},
        {"interface/doubleclicktoplay", "interface/doubleClickToPlay"},
        {"interface/clickPlaysAllSelected", "interface/clickPlaysAllSelected"},
        {"interface/playtostack", "interface/playToStack"},
        {"interface/doNotDeleteArrowsInSubPhases", "interface/doNotDeleteArrowsInSubPhases"},
        {"interface/startinghandsize", "interface/startingHandSize"},
        {"interface/annotatetokens", "interface/annotateTokens"},
        {"interface/showlassoselectioncount", "interface/showLassoSelectionCount"},
        {"interface/showpersistentselectioncount", "interface/showPersistentSelectionCount"},
        {"interface/tallyType", "interface/tallyType"},
        {"interface/leftjustified", "interface/leftJustified"},
        {"interface/min_players_multicolumn", "interface/minPlayersMulticolumn"},
        {"hand/horizontal", "hand/horizontal"},
        {"table/invert_vertical", "table/invertVertical"},
        {"replay/rewindBufferingMs", "replay/rewindBufferingMs"},
        {"replay/fastForwardSpeed", "replay/fastForwardSpeed"},
        {"zoneview/groupby", "zoneview/groupBy"},
        {"zoneview/sortby", "zoneview/sortBy"},
        {"zoneview/pileview", "zoneview/pileView"},
        {"personal/showStatusBar", "interface/showStatusBar"},
        {"menu/showshortcuts", "interface/showShortcuts"},
        {"menu/showgameselectorfiltertoolbar", "interface/showGameSelectorFilterToolbar"},
        {"interface/notificationsenabled", "interface/notifications/enabled"},
        {"interface/specnotificationsenabled", "interface/notifications/spectatorsEnabled"},
        {"interface/buddyconnectnotificationsenabled", "interface/notifications/buddyConnectEnabled"},
    };
    bool hasAny = false;
    for (auto it = interfaceKeyMap.constBegin(); it != interfaceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings interfaceIni(settingsPath + "interface.ini", QSettings::IniFormat);
    for (auto it = interfaceKeyMap.constBegin(); it != interfaceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            interfaceIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateDownloadSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> downloadKeyMap = {
        {"personal/picturedownload", "downloads/pictureDownload"},
        {"personal/downloadspoilers", "downloads/downloadSpoilers"},
    };
    bool hasAny = false;
    for (auto it = downloadKeyMap.constBegin(); it != downloadKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings downloadsIni(settingsPath + "downloads.ini", QSettings::IniFormat);
    for (auto it = downloadKeyMap.constBegin(); it != downloadKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            downloadsIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateAppearanceSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> appearanceKeyMap = {
        {"theme/name", "appearance/themeName"},
        {"game/maxfontsize", "appearance/maxFontSize"},
        {"home/background", "appearance/homeTabBackgroundSource"},
        {"home/background/shuffleTimer", "appearance/homeTabBackgroundShuffleFrequency"},
        {"home/background/displayCardName", "appearance/homeTabDisplayCardName"},
        {"appearance/styleUserList", "appearance/styleUserList"},
    };
    bool hasAny = false;
    for (auto it = appearanceKeyMap.constBegin(); it != appearanceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings appearanceIni(settingsPath + "appearance.ini", QSettings::IniFormat);
    for (auto it = appearanceKeyMap.constBegin(); it != appearanceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            QVariant value = globalIni.value(it.key());
            if (it.key() == "theme/name" && value.toString() == "Default") {
                value = "System";
            }
            appearanceIni.setValue(it.value(), value);
        }
    }
}

static void migrateNetworkSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> networkKeyMap = {
        {"personal/clientid", "network/clientId"},
        {"personal/clientversion", "network/clientVersion"},
        {"personal/keepalive", "network/keepAlive"},
        {"personal/timeout", "network/timeout"},
        {"interface/knownmissingfeatures", "network/knownMissingFeatures"},
    };
    bool hasAny = false;
    for (auto it = networkKeyMap.constBegin(); it != networkKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings networkIni(settingsPath + "network.ini", QSettings::IniFormat);
    for (auto it = networkKeyMap.constBegin(); it != networkKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            networkIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migratePathsSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> pathsKeyMap = {
        {"paths/decks", "paths/decks"},
        {"paths/filters", "paths/filters"},
        {"paths/replays", "paths/replays"},
        {"paths/pics", "paths/pics"},
        {"paths/custompics", "paths/customPics"},
        {"paths/themes", "paths/themes"},
        {"paths/carddatabase", "paths/cardDatabase"},
        {"paths/customsets", "paths/customSets"},
        {"paths/tokendatabase", "paths/tokenDatabase"},
        {"paths/spoilerdatabase", "paths/spoilerDatabase"},
        {"paths/redirects", "paths/redirects"},
    };
    bool hasAny = false;
    for (auto it = pathsKeyMap.constBegin(); it != pathsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings pathsIni(settingsPath + "paths.ini", QSettings::IniFormat);
    for (auto it = pathsKeyMap.constBegin(); it != pathsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            pathsIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateVisualDeckStorageSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> vdsKeyMap = {
        {"interface/visualdeckstoragesortingorder", "interface/visualDeckStorage/sortingOrder"},
        {"interface/visualdeckstorageshowfolders", "interface/visualDeckStorage/showFolders"},
        {"interface/visualdeckstorageshowtagfilter", "interface/visualDeckStorage/showTagFilter"},
        {"interface/visualdeckstoragedefaulttagslist", "interface/visualDeckStorage/defaultTagsList"},
        {"interface/visualdeckstoragesearchfoldernames", "interface/visualDeckStorage/searchFolderNames"},
        {"interface/visualdeckstorageshowcoloridentity", "interface/visualDeckStorage/showColorIdentity"},
        {"interface/visualdeckstorageshowbannercardcombobox", "interface/visualDeckStorage/showBannerCardComboBox"},
        {"interface/visualdeckstorageshowtagsondeckpreviews", "interface/visualDeckStorage/showTagsOnDeckPreviews"},
        {"interface/visualdeckstoragedrawunusedcoloridentities",
         "interface/visualDeckStorage/drawUnusedColorIdentities"},
        {"interface/visualdeckstorageunusedcoloridentitiesopacity",
         "interface/visualDeckStorage/unusedColorIdentitiesOpacity"},
        {"interface/visualdeckstoragetooltiptype", "interface/visualDeckStorage/tooltipType"},
        {"interface/visualdeckstoragepromptforconversion", "interface/visualDeckStorage/promptForConversion"},
        {"interface/visualdeckstoragealwaysconvert", "interface/visualDeckStorage/alwaysConvert"},
        {"interface/visualdeckstorageingame", "interface/visualDeckStorage/inGame"},
        {"interface/visualdeckstorageselectionanimation", "interface/visualDeckStorage/selectionAnimation"},
        {"interface/visualdatabasedisplayfiltertomostrecentsetsenabled",
         "interface/visualDatabaseDisplay/filterToMostRecentSetsEnabled"},
        {"interface/visualdatabasedisplayfiltertomostrecentsetsamount",
         "interface/visualDatabaseDisplay/filterToMostRecentSetsAmount"},
    };
    bool hasAny = false;
    for (auto it = vdsKeyMap.constBegin(); it != vdsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings vdsIni(settingsPath + "visual_deck_storage.ini", QSettings::IniFormat);
    for (auto it = vdsKeyMap.constBegin(); it != vdsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            vdsIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateDeckEditorSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> deckEditorKeyMap = {
        {"editor/openDeckInNewTab", "deckeditor/openDeckInNewTab"},
        {"interface/deckeditorbannercardcomboboxvisible", "deckeditor/bannerCardComboBoxVisible"},
        {"interface/deckeditortagswidgetvisible", "deckeditor/tagsWidgetVisible"},
        {"interface/defaultDeckEditorType", "deckeditor/defaultDeckEditorType"},
    };
    bool hasAny = false;
    for (auto it = deckEditorKeyMap.constBegin(); it != deckEditorKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
            break;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings deckEditorIni(settingsPath + "deck_editor.ini", QSettings::IniFormat);
    for (auto it = deckEditorKeyMap.constBegin(); it != deckEditorKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            deckEditorIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateLegacySets(const QString &settingsPath)
{
    QSettings legacySetting;
    legacySetting.beginGroup("sets");
    QStringList groups = legacySetting.childGroups();
    if (groups.isEmpty()) {
        legacySetting.endGroup();
        return;
    }

    QSettings cardDbIni(settingsPath + "cardDatabase.ini", QSettings::IniFormat);
    for (const auto &shortName : groups) {
        legacySetting.beginGroup(shortName);
        if (legacySetting.contains("sortkey")) {
            cardDbIni.setValue("sets/" + shortName + "/sortKey", legacySetting.value("sortkey"));
        }
        if (legacySetting.contains("enabled")) {
            cardDbIni.setValue("sets/" + shortName + "/enabled", legacySetting.value("enabled"));
        }
        if (legacySetting.contains("isknown")) {
            cardDbIni.setValue("sets/" + shortName + "/isKnown", legacySetting.value("isknown"));
        }
        legacySetting.endGroup();
    }
    legacySetting.endGroup();
}

static void migrateLegacyServers(const QString &settingsPath)
{
    QSettings legacySetting;
    legacySetting.beginGroup("server");
    QStringList keys = legacySetting.allKeys();
    if (keys.isEmpty()) {
        legacySetting.endGroup();
        return;
    }

    const QMap<QString, QString> serverKeyMap = {
        {"previoushostlogin", "previousHostLogin"},
        {"previoushosts", "previousHosts"},
        {"previoushostName", "previousHostName"},
        {"auto_connect", "autoConnect"},
        {"fphostname", "fpHostName"},
        {"fpport", "fpPort"},
        {"fpplayername", "fpPlayerName"},
        {"save_debug_log", "saveDebugLog"},
    };

    QSettings serversIni(settingsPath + "servers.ini", QSettings::IniFormat);
    for (auto it = serverKeyMap.constBegin(); it != serverKeyMap.constEnd(); ++it) {
        if (legacySetting.contains(it.key())) {
            serversIni.setValue("server/" + it.value(), legacySetting.value(it.key()));
        }
    }

    legacySetting.beginGroup("server_details");
    const QStringList detailsKeys = legacySetting.allKeys();
    for (const auto &key : detailsKeys) {
        serversIni.setValue("server/server_details/" + key, legacySetting.value(key));
    }
    legacySetting.endGroup();
    legacySetting.endGroup();
}

static void migrateLegacyMessages(const QString &settingsPath)
{
    QSettings legacySetting;
    legacySetting.beginGroup("messages");
    QStringList keys = legacySetting.allKeys();
    if (keys.isEmpty()) {
        legacySetting.endGroup();
        return;
    }

    QSettings messageIni(settingsPath + "messages.ini", QSettings::IniFormat);
    for (const auto &key : keys) {
        if (key == "count") {
            messageIni.setValue("messages/count", legacySetting.value("count"));
        } else {
            messageIni.setValue("messages/" + key, legacySetting.value(key));
        }
    }
    legacySetting.endGroup();
}

static void migrateLegacyGameFilters(const QString &settingsPath)
{
    QSettings legacySetting;
    legacySetting.beginGroup("filter_games");
    QStringList keys = legacySetting.allKeys();
    if (keys.isEmpty()) {
        legacySetting.endGroup();
        return;
    }

    const QMap<QString, QString> filterKeyMap = {
        {"hide_buddies_only_games", "hideBuddiesOnlyGames"},
        {"hide_full_games", "hideFullGames"},
        {"hide_games_that_started", "hideGamesThatStarted"},
        {"hide_password_protected_games", "hidePasswordProtectedGames"},
        {"hide_ignored_user_games", "hideIgnoredUserGames"},
        {"hide_not_buddy_created_games", "hideNotBuddyCreatedGames"},
        {"hide_open_decklist_games", "hideOpenDecklistGames"},
        {"game_name_filter", "gameNameFilter"},
        {"creator_name_filter", "creatorNameFilter"},
        {"min_players", "minPlayers"},
        {"max_players", "maxPlayers"},
        {"max_game_age_time", "maxGameAgeTime"},
        {"show_only_if_spectators_can_watch", "showOnlyIfSpectatorsCanWatch"},
        {"show_spectator_password_protected", "showSpectatorPasswordProtected"},
        {"show_only_if_spectators_can_chat", "showOnlyIfSpectatorsCanChat"},
        {"show_only_if_spectators_can_see_hands", "showOnlyIfSpectatorsCanSeeHands"},
    };

    QSettings filtersIni(settingsPath + "gamefilters.ini", QSettings::IniFormat);
    for (auto it = filterKeyMap.constBegin(); it != filterKeyMap.constEnd(); ++it) {
        if (legacySetting.contains(it.key())) {
            filtersIni.setValue("filter_games/" + it.value(), legacySetting.value(it.key()));
        }
    }
    const QString gameTypePrefix = "game_type/";
    for (const auto &key : keys) {
        if (key.startsWith(gameTypePrefix)) {
            filtersIni.setValue("filter_games/gameType/" + key.mid(gameTypePrefix.size()), legacySetting.value(key));
        }
    }
    legacySetting.endGroup();
}

bool SettingsMigration::migrateLegacySettings(const QString &settingsPath)
{
    QSettings personalIni(settingsPath + "personal.ini", QSettings::IniFormat);
    if (personalIni.value("migration/legacy_complete", false).toBool()) {
        return false;
    }

    migrateLegacySets(settingsPath);
    migrateLegacyServers(settingsPath);
    migrateLegacyMessages(settingsPath);
    migrateLegacyGameFilters(settingsPath);

    personalIni.setValue("migration/legacy_complete", true);
    personalIni.sync();
    return true;
}

bool SettingsMigration::migrateSettingsFromGlobalIni(const QString &settingsPath)
{
    if (!hasPendingGlobalIniSettings(settingsPath)) {
        return false;
    }

    QSettings globalIni(settingsPath + "global.ini", QSettings::IniFormat);

    migrateTabsSettings(settingsPath, globalIni);
    migrateSoundSettings(settingsPath, globalIni);
    migrateGameSettings(settingsPath, globalIni);
    migrateChatSettings(settingsPath, globalIni);
    migrateCacheStorageSettings(settingsPath, globalIni);
    migrateUpdatesSettings(settingsPath, globalIni);
    migratePersonalSettings(settingsPath, globalIni);
    migrateDownloadSettings(settingsPath, globalIni);
    migrateCardsDisplaySettings(settingsPath, globalIni);
    migrateCardCounterSettings(settingsPath, globalIni);
    migrateInterfaceSettings(settingsPath, globalIni);
    migrateAppearanceSettings(settingsPath, globalIni);
    migrateNetworkSettings(settingsPath, globalIni);
    migratePathsSettings(settingsPath, globalIni);
    migrateVisualDeckStorageSettings(settingsPath, globalIni);
    migrateDeckEditorSettings(settingsPath, globalIni);

    QFile::remove(settingsPath + "global.ini.old");
    QFile::rename(settingsPath + "global.ini", settingsPath + "global.ini.old");

    QSettings newGlobalIni(settingsPath + "global.ini", QSettings::IniFormat);
    newGlobalIni.setValue(MIGRATION_SENTINEL_KEY, true);
    newGlobalIni.sync();

    return true;
}

bool SettingsMigration::isMigrationPending(const QString &settingsPath)
{
    if (hasPendingGlobalIniSettings(settingsPath)) {
        return true;
    }

    QSettings personalIni(settingsPath + "personal.ini", QSettings::IniFormat);
    if (personalIni.value("migration/legacy_complete", false).toBool()) {
        return false;
    }

    // migrateLegacySettings() also runs on an empty profile, where it only writes its sentinel.
    // Reporting that as pending would leave a backup folder behind on every fresh install, so
    // only count it when there really are legacy keys left to move.
    return !QSettings().allKeys().isEmpty();
}

QString SettingsMigration::backupSettingsDirectory(const QString &settingsPath)
{
    QDir settingsDir(QDir::cleanPath(settingsPath));
    if (!settingsDir.exists()) {
        return QString();
    }

    const QDir::Filters fileFilters = QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

    // A profile holding nothing but empty files has nothing to lose, and backing it up would only
    // leave clutter next to the settings folder.
    const QFileInfoList files = settingsDir.entryInfoList(fileFilters);
    const bool hasContent =
        std::any_of(files.cbegin(), files.cend(), [](const QFileInfo &file) { return file.size() > 0; });
    if (!hasContent) {
        return QString();
    }

    // Keep the backup a sibling of the settings folder rather than a child: a folder that is never
    // copied into its own backup, and one the "Open settings folder" action does not have to skip.
    QDir backupParentDir(settingsDir.absolutePath());
    if (!backupParentDir.cdUp()) {
        qCWarning(SettingsMigrationLog) << "[SettingsMigration] Could not resolve the parent of the settings folder:"
                                        << settingsDir.absolutePath();
        return QString();
    }

    const QString backupName =
        QStringLiteral("settings-backup-") + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QString backupPath = backupParentDir.absoluteFilePath(backupName);
    // Two migrations inside the same second must not merge into a single folder.
    for (int suffix = 1; QFileInfo::exists(backupPath); ++suffix) {
        backupPath = backupParentDir.absoluteFilePath(backupName + '-' + QString::number(suffix));
    }

    if (!QDir().mkpath(backupPath)) {
        qCWarning(SettingsMigrationLog) << "[SettingsMigration] Could not create the settings backup folder:"
                                        << backupPath;
        return QString();
    }

    QDir backupDir(backupPath);
    QDirIterator it(settingsDir.absolutePath(), fileFilters, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();

        const QString target = backupDir.absoluteFilePath(settingsDir.relativeFilePath(it.filePath()));
        QDir().mkpath(QFileInfo(target).absolutePath());
        if (!QFile::copy(it.filePath(), target)) {
            // A partially copied backup still beats none: report it and let the migration run.
            qCWarning(SettingsMigrationLog) << "[SettingsMigration] Could not back up settings file:" << it.filePath();
        }
    }

    qCInfo(SettingsMigrationLog) << "[SettingsMigration] Settings backed up to" << backupPath;
    return backupPath;
}
