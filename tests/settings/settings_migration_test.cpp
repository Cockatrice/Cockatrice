#include "gtest/gtest.h"
#include <QColor>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QTemporaryDir>
#include <libcockatrice/settings/settings_migration.h>

namespace
{

static bool nativeSettingsAvailable()
{
    QSettings probe;
    probe.setValue("_migration_native_probe", "ok");
    probe.sync();
    QSettings read;
    bool ok = read.value("_migration_native_probe").toString() == "ok";
    QSettings().clear();
    QSettings().sync();
    return ok;
}

class SettingsMigrationTest : public ::testing::Test
{
protected:
    QTemporaryDir tempDir;
    QString settingsPath;

    void SetUp() override
    {
        settingsPath = tempDir.path() + "/";

        // Isolate native-format QSettings (used by the legacy migration tests) inside the
        // temporary directory so the tests never read or write the real user config,
        // which would otherwise be shared across CI jobs and flaky.
        const QString configDir = tempDir.path() + "/config";
        QDir().mkpath(configDir);
        qputenv("XDG_CONFIG_HOME", configDir.toUtf8());
    }

    bool fileExists(const QString &name) const
    {
        return QFile::exists(settingsPath + name);
    }

    QVariant readFromIni(const QString &fileName, const QString &key) const
    {
        QSettings ini(settingsPath + fileName, QSettings::IniFormat);
        return ini.value(key);
    }
};

TEST_F(SettingsMigrationTest, NoGlobalIniDoesNothing)
{
    bool result = SettingsMigration::migrateSettingsFromGlobalIni(settingsPath);
    ASSERT_FALSE(result);
}

TEST_F(SettingsMigrationTest, EmptyGlobalIniGetsBackedUp)
{
    {
        QFile f(settingsPath + "global.ini");
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();
    }
    bool result = SettingsMigration::migrateSettingsFromGlobalIni(settingsPath);
    ASSERT_TRUE(result);
    ASSERT_TRUE(fileExists("global.ini.old"));
    ASSERT_TRUE(fileExists("global.ini"));

    // No per-file INIs should be created
    ASSERT_FALSE(fileExists("tabs.ini"));
    ASSERT_FALSE(fileExists("sound.ini"));

    // Sentinel should be in the new global.ini
    ASSERT_EQ(readFromIni("global.ini", "migration/perfile_complete"), QVariant(true));
}

TEST_F(SettingsMigrationTest, MigratesAllSettingsGroups)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);

        // tabs
        g.setValue("tabs/visualDeckStorage", false);
        g.setValue("tabs/server", true);
        g.setValue("tabs/account", true);
        g.setValue("tabs/deckStorage", true);
        g.setValue("tabs/replays", false);
        g.setValue("tabs/admin", false);
        g.setValue("tabs/log", true);

        // sound
        g.setValue("sound/enabled", true);
        g.setValue("sound/theme", "custom_theme");
        g.setValue("sound/mastervolume", 75);

        // game
        g.beginGroup("game");
        g.setValue("maxplayers", 4);
        g.setValue("gamedescription", "test game");
        g.setValue("remembergamesettings", false);
        g.setValue("gametypes", "commander");
        g.setValue("onlybuddies", true);
        g.endGroup();

        // localgameoptions
        g.beginGroup("localgameoptions");
        g.setValue("maxplayers", 2);
        g.setValue("startinglifetotal", 40);
        g.endGroup();

        // chat
        g.beginGroup("chat");
        g.setValue("mention", false);
        g.setValue("mentioncolor", "FF0000");
        g.setValue("showmessagepopups", false);
        g.setValue("mentioncompleter", false);
        g.setValue("roomhistory", false);
        g.setValue("highlightcolor", "00FF00");
        g.endGroup();

        // legacy highlight words (under [personal])
        g.setValue("personal/highlightWords", "alpha beta");

        // cache storage (under [personal] group)
        g.setValue("personal/pixmapCacheSize", 1024);
        g.setValue("personal/networkCacheSize", 2048);
        g.setValue("personal/redirectCacheTtl", 5);
        g.setValue("personal/cardPictureLoaderCacheMethod", 1);
        g.setValue("personal/localCardImageStorageNamingScheme", 2);

        // updates (under [personal] group)
        g.setValue("personal/startupUpdateCheck", false);
        g.setValue("personal/startupCardUpdateCheckPromptForUpdate", false);
        g.setValue("personal/startupCardUpdateCheckAlwaysUpdate", true);
        g.setValue("personal/cardUpdateCheckInterval", 14);
        g.setValue("personal/lastCardUpdateCheck", QDate(2024, 1, 1));
        g.setValue("personal/alwaysEnableNewSets", true);
        g.setValue("personal/updatenotification", false);
        g.setValue("personal/newversionnotification", false);

        // personal
        g.setValue("personal/lang", "de");

        // downloads (previously under [personal])
        g.setValue("personal/picturedownload", true);
        g.setValue("personal/downloadspoilers", true);

        // interface (previously under [personal])
        g.setValue("personal/showStatusBar", true);

        // theme
        g.setValue("theme/name", "custom_theme");
        g.setValue("game/maxfontsize", 14);

        // appearance
        g.setValue("appearance/styleUserList", false);
        g.setValue("home/background/displayCardName", false);
        g.setValue("menu/showshortcuts", false);
        g.setValue("menu/showgameselectorfiltertoolbar", false);

        // deck editor
        g.setValue("editor/openDeckInNewTab", false);
        g.setValue("interface/deckeditortagswidgetvisible", false);
        g.setValue("interface/defaultDeckEditorType", 0);
        g.setValue("interface/visualdeckeditorsamplehandsize", 5);

        // personal home
        g.setValue("home/background", "custom_bg");
        g.setValue("home/background/shuffleTimer", 30);

        // personal tipOfDay
        g.setValue("tipOfDay/showTips", false);
        g.setValue("tipOfDay/seenTips", QStringList{"1", "2", "3"});

        // network
        g.setValue("personal/keepalive", 10);
        g.setValue("personal/timeout", 30);
        g.setValue("personal/clientid", "test-client-id");
        g.setValue("personal/clientversion", "test-client-version");
        g.setValue("interface/knownmissingfeatures", "feature1,feature2");

        // cards
        g.setValue("cards/displaycardnames", false);
        g.setValue("cards/roundcardcorners", false);
        g.setValue("cards/overrideallcardartwithpersonalpreference", true);
        g.setValue("cards/bumpsetswithcardsindecktotop", false);
        g.setValue("cards/includerebalancedcards", false);
        g.setValue("cards/autorotatesidewayslayoutcards", false);
        g.setValue("cards/tapanimation", true);
        g.setValue("cards/scaleCards", false);
        g.setValue("cards/verticalCardOverlapPercent", 42);
        g.setValue("cards/cardinfoviewmode", 1);
        g.setValue("cards/printingselectorcardsize", 90);
        g.setValue("cards/printingselectorsortorder", 3);
        g.setValue("cards/printingselectornavigationbuttonsvisible", false);
        // card counters (migrate into card_counters.ini)
        g.setValue("cards/counters/0/color", QColor(Qt::red));

        // interface
        g.setValue("interface/usetearoffmenus", true);
        g.setValue("interface/cardViewInitialRowsMax", 8);
        g.setValue("interface/cardViewExpandedRowsMax", 12);
        g.setValue("interface/closeEmptyCardView", false);
        g.setValue("interface/focusCardViewSearchBar", false);
        g.setValue("interface/keepGameChatFocus", true);
        g.setValue("interface/notificationsenabled", false);
        g.setValue("interface/specnotificationsenabled", true);
        g.setValue("interface/buddyconnectnotificationsenabled", false);
        g.setValue("interface/doubleclicktoplay", false);
        g.setValue("interface/clickPlaysAllSelected", false);
        g.setValue("interface/playtostack", false);
        g.setValue("interface/doNotDeleteArrowsInSubPhases", false);
        g.setValue("interface/startinghandsize", 5);
        g.setValue("interface/annotatetokens", true);
        g.setValue("interface/showlassoselectioncount", false);
        g.setValue("interface/showpersistentselectioncount", false);
        g.setValue("interface/tallyType", 2);
        g.setValue("interface/leftjustified", true);
        g.setValue("interface/min_players_multicolumn", 6);
        g.setValue("interface/deckeditorbannercardcomboboxvisible", false);
        // hand/table/replay/zoneview
        g.setValue("hand/horizontal", true);
        g.setValue("table/invert_vertical", true);
        g.setValue("replay/rewindBufferingMs", 6000);
        g.setValue("replay/fastForwardSpeed", 5);
        g.setValue("zoneview/groupby", 2);
        g.setValue("zoneview/sortby", 1);
        g.setValue("zoneview/pileview", false);

        // paths
        g.beginGroup("paths");
        g.setValue("decks", "/custom/decks");
        g.setValue("pics", "/custom/pics");
        g.endGroup();

        // visual deck storage (under [interface] group)
        g.setValue("interface/visualdeckstoragecardsize", 150);
        g.setValue("interface/visualdeckstoragesortingorder", 2);
        g.setValue("interface/visualdeckstorageshowfolders", false);
        g.setValue("interface/visualdeckstorageshowtagfilter", false);
        g.setValue("interface/visualdeckstoragedefaulttagslist", QStringList{"Alpha", "Beta"});
        g.setValue("interface/visualdeckstoragesearchfoldernames", false);
        g.setValue("interface/visualdeckstorageshowcoloridentity", false);
        g.setValue("interface/visualdeckstorageshowbannercardcombobox", false);
        g.setValue("interface/visualdeckstorageshowtagsondeckpreviews", false);
        g.setValue("interface/visualdeckstoragedrawunusedcoloridentities", false);
        g.setValue("interface/visualdeckstorageunusedcoloridentitiesopacity", 35);
        g.setValue("interface/visualdeckstoragetooltiptype", 1);
        g.setValue("interface/visualdeckstoragepromptforconversion", false);
        g.setValue("interface/visualdeckstoragealwaysconvert", true);
        g.setValue("interface/visualdeckstorageingame", false);
        g.setValue("interface/visualdeckstorageselectionanimation", false);
        g.setValue("interface/visualdatabasedisplayfiltertomostrecentsetsenabled", true);
        g.setValue("interface/visualdatabasedisplayfiltertomostrecentsetsamount", 25);

        // card sizes (migrate into cards_display.ini)
        g.setValue("interface/visualdatabasedisplaycardsize", 80);
        g.setValue("interface/visualdeckeditorcardsize", 70);
        g.setValue("interface/edhreccardsize", 60);
        g.setValue("interface/archidektpreviewsize", 50);

        g.sync();
    }

    bool result = SettingsMigration::migrateSettingsFromGlobalIni(settingsPath);
    ASSERT_TRUE(result);

    // Original global.ini should be backed up
    ASSERT_TRUE(fileExists("global.ini.old"));

    // Verify each per-file INI was created and has correct values
    ASSERT_TRUE(fileExists("tabs.ini"));
    ASSERT_EQ(readFromIni("tabs.ini", "tabs/visualDeckStorage"), QVariant(false));
    ASSERT_EQ(readFromIni("tabs.ini", "tabs/server"), QVariant(true));
    ASSERT_EQ(readFromIni("tabs.ini", "tabs/log"), QVariant(true));

    ASSERT_TRUE(fileExists("sound.ini"));
    ASSERT_EQ(readFromIni("sound.ini", "sound/enabled"), QVariant(true));
    ASSERT_EQ(readFromIni("sound.ini", "sound/theme"), QVariant("custom_theme"));
    ASSERT_EQ(readFromIni("sound.ini", "sound/masterVolume"), QVariant(75));

    ASSERT_TRUE(fileExists("game.ini"));
    ASSERT_EQ(readFromIni("game.ini", "game/maxPlayers"), QVariant(4));
    ASSERT_EQ(readFromIni("game.ini", "game/gameDescription"), QVariant("test game"));
    ASSERT_EQ(readFromIni("game.ini", "game/rememberGameSettings"), QVariant(false));
    ASSERT_EQ(readFromIni("game.ini", "game/gameTypes"), QVariant("commander"));
    ASSERT_EQ(readFromIni("game.ini", "game/onlyBuddies"), QVariant(true));
    ASSERT_EQ(readFromIni("game.ini", "localgameoptions/maxPlayers"), QVariant(2));
    ASSERT_EQ(readFromIni("game.ini", "localgameoptions/startingLifeTotal"), QVariant(40));

    ASSERT_TRUE(fileExists("chat.ini"));
    ASSERT_EQ(readFromIni("chat.ini", "chat/mention"), QVariant(false));
    ASSERT_EQ(readFromIni("chat.ini", "chat/mentionColor"), QVariant("FF0000"));
    ASSERT_EQ(readFromIni("chat.ini", "chat/showMessagePopups"), QVariant(false));
    ASSERT_EQ(readFromIni("chat.ini", "chat/mentionCompleter"), QVariant(false));
    ASSERT_EQ(readFromIni("chat.ini", "chat/roomHistory"), QVariant(false));
    ASSERT_EQ(readFromIni("chat.ini", "chat/highlightColor"), QVariant("00FF00"));
    ASSERT_EQ(readFromIni("chat.ini", "chat/highlightWords"), QVariant("alpha beta"));

    ASSERT_TRUE(fileExists("cache_storage.ini"));
    ASSERT_EQ(readFromIni("cache_storage.ini", "cache_storage/pixmapCacheSize"), QVariant(1024));
    ASSERT_EQ(readFromIni("cache_storage.ini", "cache_storage/networkCacheSize"), QVariant(2048));
    ASSERT_EQ(readFromIni("cache_storage.ini", "cache_storage/redirectCacheTtl"), QVariant(5));
    ASSERT_EQ(readFromIni("cache_storage.ini", "cache_storage/cardPictureLoaderCacheMethod"), QVariant(1));
    ASSERT_EQ(readFromIni("cache_storage.ini", "cache_storage/localCardImageStorageNamingScheme"), QVariant(2));

    ASSERT_TRUE(fileExists("updates.ini"));
    ASSERT_EQ(readFromIni("updates.ini", "updates/startupUpdateCheck"), QVariant(false));
    ASSERT_EQ(readFromIni("updates.ini", "updates/startupCardUpdateCheckPromptForUpdate"), QVariant(false));
    ASSERT_EQ(readFromIni("updates.ini", "updates/startupCardUpdateCheckAlwaysUpdate"), QVariant(true));
    ASSERT_EQ(readFromIni("updates.ini", "updates/cardUpdateCheckInterval"), QVariant(14));
    ASSERT_EQ(readFromIni("updates.ini", "updates/lastCardUpdateCheck"), QVariant(QDate(2024, 1, 1)));
    ASSERT_EQ(readFromIni("updates.ini", "updates/alwaysEnableNewSets"), QVariant(true));
    ASSERT_EQ(readFromIni("updates.ini", "updates/updateNotification"), QVariant(false));
    ASSERT_EQ(readFromIni("updates.ini", "updates/newVersionNotification"), QVariant(false));

    ASSERT_TRUE(fileExists("personal.ini"));
    ASSERT_EQ(readFromIni("personal.ini", "personal/lang"), QVariant("de"));
    ASSERT_EQ(readFromIni("personal.ini", "tipOfDay/showTips"), QVariant(false));
    ASSERT_EQ(readFromIni("personal.ini", "tipOfDay/seenTips"), QVariant(QStringList{"1", "2", "3"}));

    ASSERT_TRUE(fileExists("downloads.ini"));
    ASSERT_EQ(readFromIni("downloads.ini", "downloads/pictureDownload"), QVariant(true));
    ASSERT_EQ(readFromIni("downloads.ini", "downloads/downloadSpoilers"), QVariant(true));

    ASSERT_TRUE(fileExists("appearance.ini"));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/themeName"), QVariant("custom_theme"));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/maxFontSize"), QVariant(14));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/styleUserList"), QVariant(false));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/homeTabBackgroundSource"), QVariant("custom_bg"));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/homeTabBackgroundShuffleFrequency"), QVariant(30));
    ASSERT_EQ(readFromIni("appearance.ini", "appearance/homeTabDisplayCardName"), QVariant(false));

    ASSERT_TRUE(fileExists("network.ini"));
    ASSERT_EQ(readFromIni("network.ini", "network/keepAlive"), QVariant(10));
    ASSERT_EQ(readFromIni("network.ini", "network/timeout"), QVariant(30));
    ASSERT_EQ(readFromIni("network.ini", "network/clientId"), QVariant("test-client-id"));
    ASSERT_EQ(readFromIni("network.ini", "network/clientVersion"), QVariant("test-client-version"));
    ASSERT_EQ(readFromIni("network.ini", "network/knownMissingFeatures"), QVariant("feature1,feature2"));

    ASSERT_TRUE(fileExists("cards_display.ini"));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/displayCardNames"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/roundCardCorners"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/overrideAllCardArtWithPersonalPreference"), QVariant(true));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/bumpSetsWithCardsInDeckToTop"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/includerebalancedcards"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/autoRotateSidewaysLayoutCards"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/tapAnimation"), QVariant(true));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/scaleCards"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/verticalCardOverlapPercent"), QVariant(42));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardInfoViewMode"), QVariant(1));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/printingSelector"), QVariant(90));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/printingSelector/sortOrder"), QVariant(3));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/printingSelector/navigationButtonsVisible"), QVariant(false));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/visualDeckStorage"), QVariant(150));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/visualDatabaseDisplay"), QVariant(80));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/visualDeckEditor"), QVariant(70));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/edhrec"), QVariant(60));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/archidektPreview"), QVariant(50));
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/cardSize/sampleHandSize"), QVariant(5));

    ASSERT_TRUE(fileExists("card_counters.ini"));
    ASSERT_EQ(readFromIni("card_counters.ini", "cards/counters/0/color").toString(), QColor(Qt::red).name());
    ASSERT_FALSE(readFromIni("global.ini", "cards/counters/0/color").isValid());

    ASSERT_TRUE(fileExists("interface.ini"));
    ASSERT_EQ(readFromIni("interface.ini", "interface/useTearOffMenus"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/cardViewInitialRowsMax"), QVariant(8));
    ASSERT_EQ(readFromIni("interface.ini", "interface/cardViewExpandedRowsMax"), QVariant(12));
    ASSERT_EQ(readFromIni("interface.ini", "interface/closeEmptyCardView"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/focusCardViewSearchBar"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/keepGameChatFocus"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/notifications/enabled"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/notifications/spectatorsEnabled"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/notifications/buddyConnectEnabled"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/doubleClickToPlay"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/clickPlaysAllSelected"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/playToStack"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/doNotDeleteArrowsInSubPhases"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/startingHandSize"), QVariant(5));
    ASSERT_EQ(readFromIni("interface.ini", "interface/annotateTokens"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/showLassoSelectionCount"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/showPersistentSelectionCount"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/tallyType"), QVariant(2));
    ASSERT_EQ(readFromIni("interface.ini", "interface/leftJustified"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/minPlayersMulticolumn"), QVariant(6));
    ASSERT_EQ(readFromIni("interface.ini", "interface/showStatusBar"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/showShortcuts"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/showGameSelectorFilterToolbar"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "hand/horizontal"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "table/invertVertical"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "replay/rewindBufferingMs"), QVariant(6000));
    ASSERT_EQ(readFromIni("interface.ini", "replay/fastForwardSpeed"), QVariant(5));
    ASSERT_EQ(readFromIni("interface.ini", "zoneview/groupBy"), QVariant(2));
    ASSERT_EQ(readFromIni("interface.ini", "zoneview/sortBy"), QVariant(1));
    ASSERT_EQ(readFromIni("interface.ini", "zoneview/pileView"), QVariant(false));

    ASSERT_TRUE(fileExists("deck_editor.ini"));
    ASSERT_EQ(readFromIni("deck_editor.ini", "deckeditor/openDeckInNewTab"), QVariant(false));
    ASSERT_EQ(readFromIni("deck_editor.ini", "deckeditor/bannerCardComboBoxVisible"), QVariant(false));
    ASSERT_EQ(readFromIni("deck_editor.ini", "deckeditor/tagsWidgetVisible"), QVariant(false));
    ASSERT_EQ(readFromIni("deck_editor.ini", "deckeditor/defaultDeckEditorType"), QVariant(0));

    ASSERT_TRUE(fileExists("paths.ini"));
    ASSERT_EQ(readFromIni("paths.ini", "paths/decks"), QVariant("/custom/decks"));
    ASSERT_EQ(readFromIni("paths.ini", "paths/pics"), QVariant("/custom/pics"));

    ASSERT_TRUE(fileExists("visual_deck_storage.ini"));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/sortingOrder"), QVariant(2));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/showFolders"), QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/showTagFilter"), QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/defaultTagsList"),
              QVariant(QStringList{"Alpha", "Beta"}));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/searchFolderNames"), QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/showColorIdentity"), QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/showBannerCardComboBox"),
              QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/showTagsOnDeckPreviews"),
              QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/drawUnusedColorIdentities"),
              QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/unusedColorIdentitiesOpacity"),
              QVariant(35));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/tooltipType"), QVariant(1));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/promptForConversion"),
              QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/alwaysConvert"), QVariant(true));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/inGame"), QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDeckStorage/selectionAnimation"),
              QVariant(false));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDatabaseDisplay/filterToMostRecentSetsEnabled"),
              QVariant(true));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualDatabaseDisplay/filterToMostRecentSetsAmount"),
              QVariant(25));

    // No legacy flat keys should remain in the per-file INIs
    ASSERT_FALSE(readFromIni("visual_deck_storage.ini", "interface/visualdeckstorageshowfolders").isValid());
    ASSERT_FALSE(readFromIni("visual_deck_storage.ini", "interface/visualdeckstoragecardsize").isValid());
    ASSERT_FALSE(readFromIni("deck_editor.ini", "deckeditor/sampleHandSize").isValid());
    ASSERT_FALSE(readFromIni("deck_editor.ini", "deckeditor/cardSize").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "interface/notificationsenabled").isValid());
    ASSERT_FALSE(readFromIni("cards_display.ini", "cards/printingselectorsortorder").isValid());
    ASSERT_FALSE(readFromIni("cards_display.ini", "cards/visualDeckStorage/cardSize").isValid());

    // No legacy non-camelCase keys should remain in the per-file INIs
    ASSERT_FALSE(readFromIni("game.ini", "game/gamedescription").isValid());
    ASSERT_FALSE(readFromIni("game.ini", "localgameoptions/maxplayers").isValid());
    ASSERT_FALSE(readFromIni("chat.ini", "chat/roomhistory").isValid());
    ASSERT_FALSE(readFromIni("chat.ini", "chat/highlightwords").isValid());
    ASSERT_FALSE(readFromIni("sound.ini", "sound/mastervolume").isValid());
    ASSERT_FALSE(readFromIni("downloads.ini", "downloads/picturedownload").isValid());
    ASSERT_FALSE(readFromIni("network.ini", "network/keepalive").isValid());
    ASSERT_FALSE(readFromIni("network.ini", "network/knownmissingfeatures").isValid());
    ASSERT_FALSE(readFromIni("updates.ini", "updates/updatenotification").isValid());
    ASSERT_FALSE(readFromIni("cards_display.ini", "cards/displaycardnames").isValid());
    ASSERT_FALSE(readFromIni("cards_display.ini", "cards/cardinfoviewmode").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "interface/usetearoffmenus").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "interface/doubleclicktoplay").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "interface/min_players_multicolumn").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "table/invert_vertical").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "zoneview/groupby").isValid());
    ASSERT_FALSE(readFromIni("interface.ini", "zoneview/pileview").isValid());

    // Verify sentinel was written
    ASSERT_EQ(readFromIni("global.ini", "migration/perfile_complete"), QVariant(true));
}

TEST_F(SettingsMigrationTest, SecondCallDoesNothing)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("tabs/visualDeckStorage", true);
        g.setValue("sound/enabled", true);
        g.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));
    ASSERT_FALSE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));
}

TEST_F(SettingsMigrationTest, SecondCallDoesNotOverwriteUserChanges)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("tabs/visualDeckStorage", true);
        g.setValue("sound/enabled", true);
        g.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    // User changes the value after migration
    {
        QSettings tabsIni(settingsPath + "tabs.ini", QSettings::IniFormat);
        tabsIni.setValue("tabs/visualDeckStorage", false);
        tabsIni.sync();
    }

    // Second migration should NOT overwrite the user's change
    ASSERT_FALSE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    ASSERT_EQ(readFromIni("tabs.ini", "tabs/visualDeckStorage"), QVariant(false));
}

TEST_F(SettingsMigrationTest, PartialMigrationOnlyMigratesPresentKeys)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("tabs/visualDeckStorage", true);
        g.setValue("tabs/server", false);
        g.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    ASSERT_TRUE(fileExists("global.ini.old"));
    ASSERT_TRUE(fileExists("tabs.ini"));
    ASSERT_EQ(readFromIni("tabs.ini", "tabs/visualDeckStorage"), QVariant(true));
    ASSERT_EQ(readFromIni("tabs.ini", "tabs/server"), QVariant(false));

    // Other files should NOT exist since no keys were present for them
    ASSERT_FALSE(fileExists("sound.ini"));
    ASSERT_FALSE(fileExists("game.ini"));
    ASSERT_FALSE(fileExists("chat.ini"));
}

TEST_F(SettingsMigrationTest, KeyMapTranslationIsCorrect)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("personal/cardUpdateCheckInterval", 30);
        g.setValue("personal/updatereleasechannel", 1);
        g.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    ASSERT_TRUE(fileExists("global.ini.old"));
    ASSERT_TRUE(fileExists("updates.ini"));
    // The key should be translated from "personal/cardUpdateCheckInterval" to "updates/cardUpdateCheckInterval"
    ASSERT_EQ(readFromIni("updates.ini", "updates/cardUpdateCheckInterval"), QVariant(30));
    ASSERT_EQ(readFromIni("updates.ini", "updates/updateReleaseChannel"), QVariant(1));
}

TEST_F(SettingsMigrationTest, CardsKeysKeepGroupPrefix)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("cards/displaycardnames", false);
        g.setValue("interface/deckeditorbannercardcomboboxvisible", true);
        g.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    ASSERT_TRUE(fileExists("global.ini.old"));
    ASSERT_TRUE(fileExists("cards_display.ini"));
    ASSERT_TRUE(fileExists("deck_editor.ini"));
    // "cards/displaycardnames" should be stored with its group prefix
    ASSERT_EQ(readFromIni("cards_display.ini", "cards/displayCardNames"), QVariant(false));
    // deck editor keys belong to the deck editor settings now
    ASSERT_EQ(readFromIni("deck_editor.ini", "deckeditor/bannerCardComboBoxVisible"), QVariant(true));
}

TEST_F(SettingsMigrationTest, LegacyMigrationIsIdempotent)
{
    if (!nativeSettingsAvailable()) {
        GTEST_SKIP() << "NativeFormat QSettings not available in this environment";
    }

    {
        QSettings nativeSettings;
        nativeSettings.setValue("server/previoushostlogin", "test_user");
        nativeSettings.setValue("server/auto_connect", true);
        nativeSettings.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateLegacySettings(settingsPath));

    // Change the migrated value
    {
        QSettings serversIni(settingsPath + "servers.ini", QSettings::IniFormat);
        serversIni.setValue("server/previousHostLogin", "modified_user");
        serversIni.sync();
    }

    // Second migration should NOT overwrite the change
    ASSERT_FALSE(SettingsMigration::migrateLegacySettings(settingsPath));
    ASSERT_EQ(readFromIni("servers.ini", "server/previousHostLogin"), QVariant("modified_user"));
}

TEST_F(SettingsMigrationTest, LegacyMigrationCamelCasesKeys)
{
    if (!nativeSettingsAvailable()) {
        GTEST_SKIP() << "NativeFormat QSettings not available in this environment";
    }

    {
        QSettings nativeSettings;
        nativeSettings.setValue("sets/AAA/sortkey", 2);
        nativeSettings.setValue("sets/AAA/enabled", false);
        nativeSettings.setValue("sets/AAA/isknown", false);
        nativeSettings.setValue("server/previoushostlogin", "legacy_user");
        nativeSettings.setValue("server/auto_connect", 1);
        nativeSettings.setValue("server/fpport", "5080");
        nativeSettings.setValue("messages/count", 1);
        nativeSettings.setValue("messages/msg0", "hello");
        nativeSettings.setValue("filter_games/hide_full_games", true);
        nativeSettings.setValue("filter_games/min_players", 3);
        nativeSettings.setValue("filter_games/max_players", 5);
        nativeSettings.setValue("filter_games/game_type/deadbeef", true);
        nativeSettings.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateLegacySettings(settingsPath));

    ASSERT_TRUE(fileExists("cardDatabase.ini"));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/AAA/sortKey"), QVariant(2));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/AAA/enabled"), QVariant(false));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/AAA/isKnown"), QVariant(false));

    ASSERT_TRUE(fileExists("servers.ini"));
    ASSERT_EQ(readFromIni("servers.ini", "server/previousHostLogin"), QVariant("legacy_user"));
    ASSERT_EQ(readFromIni("servers.ini", "server/autoConnect"), QVariant(1));
    ASSERT_EQ(readFromIni("servers.ini", "server/fpPort"), QVariant("5080"));

    ASSERT_TRUE(fileExists("messages.ini"));
    ASSERT_EQ(readFromIni("messages.ini", "messages/count"), QVariant(1));
    ASSERT_EQ(readFromIni("messages.ini", "messages/msg0"), QVariant("hello"));

    ASSERT_TRUE(fileExists("gamefilters.ini"));
    ASSERT_EQ(readFromIni("gamefilters.ini", "filter_games/hideFullGames"), QVariant(true));
    ASSERT_EQ(readFromIni("gamefilters.ini", "filter_games/minPlayers"), QVariant(3));
    ASSERT_EQ(readFromIni("gamefilters.ini", "filter_games/maxPlayers"), QVariant(5));
    ASSERT_EQ(readFromIni("gamefilters.ini", "filter_games/gameType/deadbeef"), QVariant(true));
}

TEST_F(SettingsMigrationTest, LegacyMigrationEmptyNativeFormatWritesSentinel)
{
    if (nativeSettingsAvailable()) {
        QSettings().clear();
        QSettings().sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateLegacySettings(settingsPath));
    ASSERT_TRUE(fileExists("personal.ini"));
    ASSERT_EQ(readFromIni("personal.ini", "migration/legacy_complete"), QVariant(true));
    ASSERT_FALSE(fileExists("messages.ini"));
    ASSERT_FALSE(fileExists("gamefilters.ini"));
    ASSERT_FALSE(fileExists("cardDatabase.ini"));

    // Second call should be a no-op
    ASSERT_FALSE(SettingsMigration::migrateLegacySettings(settingsPath));
}

// --- migrateSettingsFromGlobalIni rename-failure test ---

TEST_F(SettingsMigrationTest, RenameFailureWithExistingBackup)
{
    {
        QSettings g(settingsPath + "global.ini", QSettings::IniFormat);
        g.setValue("tabs/visualDeckStorage", true);
        g.setValue("sound/enabled", true);
        g.sync();
    }

    // Create a stale backup
    {
        QFile oldBackup(settingsPath + "global.ini.old");
        ASSERT_TRUE(oldBackup.open(QIODevice::WriteOnly));
        oldBackup.close();
    }

    // Migration should still succeed
    ASSERT_TRUE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));

    // Per-file INIs should be created
    ASSERT_TRUE(fileExists("tabs.ini"));
    ASSERT_TRUE(fileExists("sound.ini"));

    // Sentinel should be set in the new global.ini
    ASSERT_EQ(readFromIni("global.ini", "migration/perfile_complete"), QVariant(true));

    // Original data should be backed up (stale backup was replaced)
    ASSERT_TRUE(fileExists("global.ini.old"));
    ASSERT_EQ(readFromIni("global.ini.old", "tabs/visualDeckStorage"), QVariant(true));

    // Second call should be a no-op
    ASSERT_FALSE(SettingsMigration::migrateSettingsFromGlobalIni(settingsPath));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    QCoreApplication app(argc, argv);
    app.setOrganizationName("CockatriceTest");
    app.setApplicationName("SettingsMigrationTest");
    return RUN_ALL_TESTS();
}
