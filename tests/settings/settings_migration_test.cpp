#include "gtest/gtest.h"
#include <QCoreApplication>
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
        g.endGroup();

        // cache storage (under [personal] group)
        g.setValue("personal/pixmapCacheSize", 1024);
        g.setValue("personal/networkCacheSize", 2048);

        // updates (under [personal] group)
        g.setValue("personal/startupUpdateCheck", false);
        g.setValue("personal/cardUpdateCheckInterval", 14);

        // personal
        g.setValue("personal/lang", "de");
        g.setValue("personal/keepalive", 10);
        g.setValue("personal/timeout", 30);
        g.setValue("personal/clientid", "test-client-id");
        g.setValue("personal/picturedownload", true);
        g.setValue("personal/showStatusBar", true);

        // personal home
        g.setValue("home/background", "custom_bg");
        g.setValue("home/background/shuffleTimer", 30);

        // personal tipOfDay
        g.setValue("tipOfDay/showTips", false);

        // cards
        g.setValue("cards/displaycardnames", false);
        g.setValue("cards/tapanimation", true);
        g.setValue("cards/scaleCards", false);

        // interface
        g.setValue("interface/usetearoffmenus", true);
        g.setValue("interface/notificationsenabled", false);
        g.setValue("interface/startinghandsize", 5);

        // hand/table
        g.setValue("hand/horizontal", true);
        g.setValue("table/invert_vertical", true);

        // paths
        g.beginGroup("paths");
        g.setValue("decks", "/custom/decks");
        g.setValue("pics", "/custom/pics");
        g.endGroup();

        // visual deck storage (under [interface] group)
        g.setValue("interface/visualdeckstoragecardsize", 150);
        g.setValue("interface/visualdeckstorageshowfolders", false);
        g.setValue("interface/visualdeckstorageshowtagfilter", false);

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
    ASSERT_EQ(readFromIni("sound.ini", "sound/mastervolume"), QVariant(75));

    ASSERT_TRUE(fileExists("game.ini"));
    ASSERT_EQ(readFromIni("game.ini", "game/maxplayers"), QVariant(4));
    ASSERT_EQ(readFromIni("game.ini", "game/gamedescription"), QVariant("test game"));
    ASSERT_EQ(readFromIni("game.ini", "game/remembergamesettings"), QVariant(false));
    ASSERT_EQ(readFromIni("game.ini", "localgameoptions/maxplayers"), QVariant(2));
    ASSERT_EQ(readFromIni("game.ini", "localgameoptions/startinglifetotal"), QVariant(40));

    ASSERT_TRUE(fileExists("chat.ini"));
    ASSERT_EQ(readFromIni("chat.ini", "chat/mention"), QVariant(false));
    ASSERT_EQ(readFromIni("chat.ini", "chat/mentioncolor"), QVariant("FF0000"));
    ASSERT_EQ(readFromIni("chat.ini", "chat/showmessagepopups"), QVariant(false));

    ASSERT_TRUE(fileExists("cache_storage.ini"));
    ASSERT_EQ(readFromIni("cache_storage.ini", "personal/pixmapCacheSize"), QVariant(1024));
    ASSERT_EQ(readFromIni("cache_storage.ini", "personal/networkCacheSize"), QVariant(2048));

    ASSERT_TRUE(fileExists("updates.ini"));
    ASSERT_EQ(readFromIni("updates.ini", "updates/startupUpdateCheck"), QVariant(false));
    ASSERT_EQ(readFromIni("updates.ini", "updates/cardUpdateCheckInterval"), QVariant(14));

    ASSERT_TRUE(fileExists("personal.ini"));
    ASSERT_EQ(readFromIni("personal.ini", "personal/lang"), QVariant("de"));
    ASSERT_EQ(readFromIni("personal.ini", "personal/keepalive"), QVariant(10));
    ASSERT_EQ(readFromIni("personal.ini", "personal/clientid"), QVariant("test-client-id"));
    ASSERT_EQ(readFromIni("personal.ini", "personal/showStatusBar"), QVariant(true));
    ASSERT_EQ(readFromIni("personal.ini", "home/background"), QVariant("custom_bg"));
    ASSERT_EQ(readFromIni("personal.ini", "tipOfDay/showTips"), QVariant(false));

    ASSERT_TRUE(fileExists("cards.ini"));
    ASSERT_EQ(readFromIni("cards.ini", "cards/displaycardnames"), QVariant(false));
    ASSERT_EQ(readFromIni("cards.ini", "cards/tapanimation"), QVariant(true));

    ASSERT_TRUE(fileExists("interface.ini"));
    ASSERT_EQ(readFromIni("interface.ini", "interface/usetearoffmenus"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "interface/notificationsenabled"), QVariant(false));
    ASSERT_EQ(readFromIni("interface.ini", "interface/startinghandsize"), QVariant(5));
    ASSERT_EQ(readFromIni("interface.ini", "hand/horizontal"), QVariant(true));
    ASSERT_EQ(readFromIni("interface.ini", "table/invert_vertical"), QVariant(true));

    ASSERT_TRUE(fileExists("paths.ini"));
    ASSERT_EQ(readFromIni("paths.ini", "paths/decks"), QVariant("/custom/decks"));
    ASSERT_EQ(readFromIni("paths.ini", "paths/pics"), QVariant("/custom/pics"));

    ASSERT_TRUE(fileExists("visual_deck_storage.ini"));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualdeckstoragecardsize"), QVariant(150));
    ASSERT_EQ(readFromIni("visual_deck_storage.ini", "interface/visualdeckstorageshowfolders"), QVariant(false));

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
    ASSERT_EQ(readFromIni("updates.ini", "updates/updatereleasechannel"), QVariant(1));
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
    ASSERT_TRUE(fileExists("cards.ini"));
    // "cards/displaycardnames" should be stored with its group prefix
    ASSERT_EQ(readFromIni("cards.ini", "cards/displaycardnames"), QVariant(false));
    // "interface/..." keys should keep their full path
    ASSERT_EQ(readFromIni("cards.ini", "interface/deckeditorbannercardcomboboxvisible"), QVariant(true));
}

// --- migrateLegacySettings tests ---

TEST_F(SettingsMigrationTest, LegacyMigratesServersMessagesAndFilters)
{
    if (!nativeSettingsAvailable()) {
        GTEST_SKIP() << "NativeFormat QSettings not available in this environment";
    }

    {
        QSettings nativeSettings;
        nativeSettings.setValue("server/previoushostlogin", "test_user");
        nativeSettings.setValue("server/auto_connect", true);
        nativeSettings.beginGroup("messages");
        nativeSettings.setValue("count", 1);
        nativeSettings.setValue("message0", "hello");
        nativeSettings.endGroup();
        nativeSettings.beginGroup("filter_games");
        nativeSettings.setValue("show_empty", true);
        nativeSettings.endGroup();
        nativeSettings.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateLegacySettings(settingsPath));

    ASSERT_TRUE(fileExists("servers.ini"));
    ASSERT_EQ(readFromIni("servers.ini", "server/previoushostlogin"), QVariant("test_user"));
    ASSERT_EQ(readFromIni("servers.ini", "server/auto_connect"), QVariant(true));

    ASSERT_TRUE(fileExists("messages.ini"));
    ASSERT_EQ(readFromIni("messages.ini", "messages/count"), QVariant(1));
    ASSERT_EQ(readFromIni("messages.ini", "messages/message0"), QVariant("hello"));

    ASSERT_TRUE(fileExists("gamefilters.ini"));
    ASSERT_EQ(readFromIni("gamefilters.ini", "filter_games/show_empty"), QVariant(true));
}

TEST_F(SettingsMigrationTest, LegacyMigratesSets)
{
    if (!nativeSettingsAvailable()) {
        GTEST_SKIP() << "NativeFormat QSettings not available in this environment";
    }

    {
        QSettings nativeSettings;
        nativeSettings.beginGroup("sets");
        nativeSettings.beginGroup("set1");
        nativeSettings.setValue("sortkey", "001");
        nativeSettings.setValue("enabled", true);
        nativeSettings.setValue("isknown", false);
        nativeSettings.endGroup();
        nativeSettings.endGroup();
        nativeSettings.sync();
    }

    ASSERT_TRUE(SettingsMigration::migrateLegacySettings(settingsPath));

    ASSERT_TRUE(fileExists("cardDatabase.ini"));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/set1/sortkey"), QVariant("001"));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/set1/enabled"), QVariant(true));
    ASSERT_EQ(readFromIni("cardDatabase.ini", "sets/set1/isknown"), QVariant(false));
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
        serversIni.setValue("server/previoushostlogin", "modified_user");
        serversIni.sync();
    }

    // Second migration should NOT overwrite the change
    ASSERT_FALSE(SettingsMigration::migrateLegacySettings(settingsPath));
    ASSERT_EQ(readFromIni("servers.ini", "server/previoushostlogin"), QVariant("modified_user"));
}

TEST_F(SettingsMigrationTest, LegacyMigrationEmptyNativeFormatWritesSentinel)
{
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
