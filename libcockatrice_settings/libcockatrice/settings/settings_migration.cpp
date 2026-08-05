#include "settings_migration.h"

#include <QFile>
#include <QMap>
#include <QSettings>
#include <QStringList>

static const QString MIGRATION_SENTINEL_KEY = QStringLiteral("migration/perfile_complete");

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
    soundIni.setValue("sound/mastervolume", globalIni.value("sound/mastervolume", 100));
}

static void migrateGameSettings(const QString &settingsPath, QSettings &globalIni)
{
    bool hasGameKeys = false;
    globalIni.beginGroup("game");
    if (!globalIni.childKeys().isEmpty()) {
        hasGameKeys = true;
    }
    QStringList gameKeys = globalIni.childKeys();
    globalIni.endGroup();

    globalIni.beginGroup("localgameoptions");
    if (!globalIni.childKeys().isEmpty()) {
        hasGameKeys = true;
    }
    QStringList localGameKeys = globalIni.childKeys();
    globalIni.endGroup();

    if (!hasGameKeys) {
        return;
    }

    QSettings gameIni(settingsPath + "game.ini", QSettings::IniFormat);
    for (const auto &key : gameKeys) {
        if (key == "maxfontsize") {
            continue;
        }
        gameIni.setValue("game/" + key, globalIni.value("game/" + key));
    }
    for (const auto &key : localGameKeys) {
        gameIni.setValue("localgameoptions/" + key, globalIni.value("localgameoptions/" + key));
    }
}

static void migrateChatSettings(const QString &settingsPath, QSettings &globalIni)
{
    globalIni.beginGroup("chat");
    QStringList chatKeys = globalIni.childKeys();
    globalIni.endGroup();

    // Legacy highlight words lived under [personal], but the chat settings
    // class reads them from [chat]
    bool hasHighlightWords = globalIni.contains("personal/highlightWords");
    if (chatKeys.isEmpty() && !hasHighlightWords) {
        return;
    }

    QSettings chatIni(settingsPath + "chat.ini", QSettings::IniFormat);
    for (const auto &key : chatKeys) {
        chatIni.setValue("chat/" + key, globalIni.value("chat/" + key));
    }
    if (hasHighlightWords) {
        chatIni.setValue("chat/highlightwords", globalIni.value("personal/highlightWords"));
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
        {"personal/updatenotification", "updates/updatenotification"},
        {"personal/newversionnotification", "updates/newversionnotification"},
        {"personal/updatereleasechannel", "updates/updatereleasechannel"},
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
    const QStringList cardsRootKeys = {
        "cards/displaycardnames",
        "cards/roundcardcorners",
        "cards/overrideallcardartwithpersonalpreference",
        "cards/bumpsetswithcardsindecktotop",
        "cards/includerebalancedcards",
        "cards/tapanimation",
        "cards/autorotatesidewayslayoutcards",
        "cards/scaleCards",
        "cards/verticalCardOverlapPercent",
        "cards/cardinfoviewmode",
    };
    const QMap<QString, QString> cardsKeyMap = {
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
    for (const auto &key : cardsRootKeys) {
        if (globalIni.contains(key)) {
            hasAny = true;
        }
    }
    for (auto it = cardsKeyMap.constBegin(); it != cardsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings cardsIni(settingsPath + "cards_display.ini", QSettings::IniFormat);
    for (const auto &key : cardsRootKeys) {
        if (globalIni.contains(key)) {
            cardsIni.setValue(key, globalIni.value(key));
        }
    }
    for (auto it = cardsKeyMap.constBegin(); it != cardsKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            cardsIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateInterfaceSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QStringList interfaceRootKeys = {
        "interface/usetearoffmenus",
        "interface/cardViewInitialRowsMax",
        "interface/cardViewExpandedRowsMax",
        "interface/closeEmptyCardView",
        "interface/focusCardViewSearchBar",
        "interface/keepGameChatFocus",
        "interface/doubleclicktoplay",
        "interface/clickPlaysAllSelected",
        "interface/playtostack",
        "interface/doNotDeleteArrowsInSubPhases",
        "interface/startinghandsize",
        "interface/annotatetokens",
        "interface/showlassoselectioncount",
        "interface/showpersistentselectioncount",
        "interface/tallyType",
        "interface/leftjustified",
        "interface/min_players_multicolumn",
    };
    const QStringList interfaceSubKeys = {
        "hand/horizontal",  "table/invert_vertical", "replay/rewindBufferingMs", "replay/fastForwardSpeed",
        "zoneview/groupby", "zoneview/sortby",       "zoneview/pileview"};
    const QMap<QString, QString> interfaceKeyMap = {
        {"personal/showStatusBar", "interface/showStatusBar"},
        {"menu/showshortcuts", "interface/showShortcuts"},
        {"menu/showgameselectorfiltertoolbar", "interface/showGameSelectorFilterToolbar"},
        {"interface/notificationsenabled", "interface/notifications/enabled"},
        {"interface/specnotificationsenabled", "interface/notifications/spectatorsEnabled"},
        {"interface/buddyconnectnotificationsenabled", "interface/notifications/buddyConnectEnabled"},
    };
    bool hasAny = false;
    for (const auto &key : interfaceRootKeys) {
        if (globalIni.contains(key)) {
            hasAny = true;
        }
    }
    for (const auto &key : interfaceSubKeys) {
        if (globalIni.contains(key)) {
            hasAny = true;
        }
    }
    for (auto it = interfaceKeyMap.constBegin(); it != interfaceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            hasAny = true;
        }
    }
    if (!hasAny) {
        return;
    }

    QSettings interfaceIni(settingsPath + "interface.ini", QSettings::IniFormat);
    for (const auto &key : interfaceRootKeys) {
        if (globalIni.contains(key)) {
            interfaceIni.setValue(key, globalIni.value(key));
        }
    }
    for (const auto &key : interfaceSubKeys) {
        if (globalIni.contains(key)) {
            interfaceIni.setValue(key, globalIni.value(key));
        }
    }
    for (auto it = interfaceKeyMap.constBegin(); it != interfaceKeyMap.constEnd(); ++it) {
        if (globalIni.contains(it.key())) {
            interfaceIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateDownloadSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> downloadKeyMap = {
        {"personal/picturedownload", "downloads/picturedownload"},
        {"personal/downloadspoilers", "downloads/downloadspoilers"},
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
            appearanceIni.setValue(it.value(), globalIni.value(it.key()));
        }
    }
}

static void migrateNetworkSettings(const QString &settingsPath, QSettings &globalIni)
{
    const QMap<QString, QString> networkKeyMap = {
        {"personal/clientid", "network/clientid"},
        {"personal/clientversion", "network/clientversion"},
        {"personal/keepalive", "network/keepalive"},
        {"personal/timeout", "network/timeout"},
        {"interface/knownmissingfeatures", "network/knownmissingfeatures"},
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
    globalIni.beginGroup("paths");
    QStringList pathsKeys = globalIni.childKeys();
    globalIni.endGroup();
    if (pathsKeys.isEmpty()) {
        return;
    }

    QSettings pathsIni(settingsPath + "paths.ini", QSettings::IniFormat);
    for (const auto &key : pathsKeys) {
        pathsIni.setValue("paths/" + key, globalIni.value("paths/" + key));
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
        cardDbIni.setValue("sets/" + shortName + "/sortkey", legacySetting.value("sortkey"));
        cardDbIni.setValue("sets/" + shortName + "/enabled", legacySetting.value("enabled"));
        cardDbIni.setValue("sets/" + shortName + "/isknown", legacySetting.value("isknown"));
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

    QSettings serversIni(settingsPath + "servers.ini", QSettings::IniFormat);
    serversIni.setValue("server/previoushostlogin", legacySetting.value("previoushostlogin"));
    serversIni.setValue("server/previoushosts", legacySetting.value("previoushosts"));
    serversIni.setValue("server/auto_connect", legacySetting.value("auto_connect"));
    serversIni.setValue("server/fphostname", legacySetting.value("fphostname"));
    serversIni.setValue("server/fpport", legacySetting.value("fpport"));
    serversIni.setValue("server/fpplayername", legacySetting.value("fpplayername"));
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

    QSettings filtersIni(settingsPath + "gamefilters.ini", QSettings::IniFormat);
    for (const auto &key : keys) {
        filtersIni.setValue("filter_games/" + key, legacySetting.value(key));
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
    if (!QFile::exists(settingsPath + "global.ini")) {
        return false;
    }

    QSettings globalIni(settingsPath + "global.ini", QSettings::IniFormat);
    if (globalIni.value(MIGRATION_SENTINEL_KEY, false).toBool()) {
        // If a user runs an older Cockatrice build that writes to global.ini (non-sentinel keys)
        // and then upgrades back, those new keys will be silently ignored. Re-migrate them.
        globalIni.sync();
        auto allKeys = globalIni.allKeys();
        allKeys.removeAll(MIGRATION_SENTINEL_KEY);
        if (allKeys.isEmpty()) {
            return false;
        }
    }

    migrateTabsSettings(settingsPath, globalIni);
    migrateSoundSettings(settingsPath, globalIni);
    migrateGameSettings(settingsPath, globalIni);
    migrateChatSettings(settingsPath, globalIni);
    migrateCacheStorageSettings(settingsPath, globalIni);
    migrateUpdatesSettings(settingsPath, globalIni);
    migratePersonalSettings(settingsPath, globalIni);
    migrateDownloadSettings(settingsPath, globalIni);
    migrateCardsDisplaySettings(settingsPath, globalIni);
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
