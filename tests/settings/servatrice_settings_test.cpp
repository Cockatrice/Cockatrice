#include "settingscache.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QString>
#include <QTemporaryDir>
#include <gtest/gtest.h>

namespace
{
/**
 * QSettings' path helpers and the portable-mode probe need a QCoreApplication.
 * The test binary keeps one alive for the whole process.
 */
QCoreApplication &testApplication()
{
    static int argc = 1;
    static char name[] = "servatrice_settings_test";
    static char *argv[] = {name, nullptr};
    static QCoreApplication application(argc, argv);
    return application;
}

class ServatriceSettingsTest : public ::testing::Test
{
protected:
    QTemporaryDir dir;

    void SetUp() override
    {
        testApplication();
        ASSERT_TRUE(dir.isValid());
    }

    QString configPath() const
    {
        return dir.filePath(QStringLiteral("servatrice.ini"));
    }

    void writeConfig(const QByteArray &contents) const
    {
        QFile file(configPath());
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        ASSERT_EQ(file.write(contents), contents.size());
    }
};
} // namespace

TEST_F(ServatriceSettingsTest, ReadsValueFromIni)
{
    writeConfig("[server]\nport=4747\n");
    SettingsCache settings(configPath());

    EXPECT_EQ(settings.value("server/port").toInt(), 4747);
}

TEST_F(ServatriceSettingsTest, MissingKeyFallsBackToDefault)
{
    writeConfig("[server]\nport=4747\n");
    SettingsCache settings(configPath());

    EXPECT_EQ(settings.value("server/missing", 1234).toInt(), 1234);
}

TEST_F(ServatriceSettingsTest, OverrideWinsWithoutTouchingTheFile)
{
    writeConfig("[server]\nport=4747\n");
    SettingsCache settings(configPath());
    settings.setOverride("server/port", 1234);

    EXPECT_EQ(settings.value("server/port").toInt(), 1234);
    // The base class still sees the file value: an override is per-run only.
    EXPECT_EQ(static_cast<QSettings &>(settings).value("server/port").toInt(), 4747);
}

TEST_F(ServatriceSettingsTest, OverrideAppearsWhenKeyIsAbsentFromTheFile)
{
    writeConfig("[server]\nport=4747\n");
    SettingsCache settings(configPath());
    settings.setOverride("server/name", "override-only");

    EXPECT_EQ(settings.value("server/name").toString(), QStringLiteral("override-only"));
    EXPECT_FALSE(settings.allKeys().contains("server/name"));
    EXPECT_TRUE(settings.getOverrides().contains("server/name"));
}

TEST_F(ServatriceSettingsTest, RebuildUsesTheOverriddenRegexpList)
{
    writeConfig("[users]\ndisallowedregexp=^bad$\n");
    SettingsCache settings(configPath());
    ASSERT_EQ(settings.disallowedRegExp.size(), 1);
    EXPECT_TRUE(settings.disallowedRegExp.first().match("bad").hasMatch());

    settings.setOverride("users/disallowedregexp", "^worse$");
    settings.rebuildDisallowedRegExps();

    ASSERT_EQ(settings.disallowedRegExp.size(), 1);
    EXPECT_TRUE(settings.disallowedRegExp.first().match("worse").hasMatch());
    EXPECT_FALSE(settings.disallowedRegExp.first().match("bad").hasMatch());
}

TEST_F(ServatriceSettingsTest, EffectiveKeysIncludeOverridesAndStaySorted)
{
    writeConfig("[server]\nport=4747\n");
    SettingsCache settings(configPath());
    settings.setOverride("server/name", "override-only");

    const QStringList keys = settings.effectiveKeys();
    EXPECT_TRUE(keys.contains("server/name"));
    EXPECT_TRUE(keys.contains("server/port"));

    QStringList sorted = keys;
    sorted.sort();
    EXPECT_EQ(keys, sorted);
}

TEST(ServatriceSettingsParsingTest, SecretKeysAreRecognised)
{
    EXPECT_TRUE(SettingsCache::isSecretKey("authentication/password"));
    EXPECT_TRUE(SettingsCache::isSecretKey("database/password"));
    EXPECT_TRUE(SettingsCache::isSecretKey("smtp/password"));
    EXPECT_TRUE(SettingsCache::isSecretKey("security/legacy_backup_key"));
    EXPECT_TRUE(SettingsCache::isSecretKey("server/apiSecret"));
    EXPECT_FALSE(SettingsCache::isSecretKey("server/port"));
}

TEST(ServatriceSettingsParsingTest, ParseOverrideAcceptsScalarKeys)
{
    QString key;
    QVariant value;

    ASSERT_TRUE(SettingsCache::parseOverride("server/port=14747", key, value));
    EXPECT_EQ(key, QStringLiteral("server/port"));
    EXPECT_EQ(value.toString(), QStringLiteral("14747"));

    // Only the first '=' separates the key; the rest belongs to the value.
    ASSERT_TRUE(SettingsCache::parseOverride("server/name=a=b", key, value));
    EXPECT_EQ(value.toString(), QStringLiteral("a=b"));
}

TEST(ServatriceSettingsParsingTest, ParseOverrideRejectsMalformedEntries)
{
    QString key;
    QVariant value;

    EXPECT_FALSE(SettingsCache::parseOverride("server/port", key, value));
    EXPECT_FALSE(SettingsCache::parseOverride("=14747", key, value));
    EXPECT_FALSE(SettingsCache::parseOverride("   =14747", key, value));
}

TEST(ServatriceSettingsParsingTest, ParseOverrideRejectsBareAndArrayScopedKeys)
{
    QString key;
    QVariant value;

    EXPECT_FALSE(SettingsCache::parseOverride("name=Kitchen", key, value));
    EXPECT_FALSE(SettingsCache::parseOverride("rooms/roomlist/1/name=Kitchen", key, value));
    EXPECT_FALSE(SettingsCache::parseOverride("rooms/roomlist/01/name=Kitchen", key, value));
    EXPECT_FALSE(SettingsCache::parseOverride("rooms/roomlist/size=2", key, value));
}
