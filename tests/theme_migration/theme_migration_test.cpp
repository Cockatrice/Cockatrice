#include "interface/theme_manager.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <string>

// Owned by main.cpp in the real application; pixel_map_generator.cpp references
// it, so the test binary needs a definition even though it never renders.
ThemeManager *themeManager = nullptr;

namespace
{
const QString LEGACY = QStringLiteral("Default");
const QString CURRENT = QStringLiteral("System");

// Writes content at <themesPath>/<theme>/<relativePath>, creating parents.
void writeThemeFile(const QString &themesPath,
                    const QString &theme,
                    const QString &relativePath,
                    const QByteArray &content)
{
    const QString filePath = QDir(QDir(themesPath).absoluteFilePath(theme)).absoluteFilePath(relativePath);
    ASSERT_TRUE(QDir().mkpath(QFileInfo(filePath).absolutePath()));
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(content);
    file.close();
}

QByteArray readThemeFile(const QString &themesPath, const QString &theme, const QString &relativePath)
{
    QFile file(QDir(QDir(themesPath).absoluteFilePath(theme)).absoluteFilePath(relativePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

class ThemeMigrationTest : public ::testing::Test
{
protected:
    QTemporaryDir tmp;
    QString themesPath;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        themesPath = QDir(tmp.path()).absoluteFilePath("themes");
        ASSERT_TRUE(QDir().mkpath(themesPath));
    }
};

} // namespace

TEST_F(ThemeMigrationTest, NoLegacyDirectoryIsANoOp)
{
    writeThemeFile(themesPath, CURRENT, "theme.cfg", "[Appearance]\n");

    ThemeManager::migrateLegacyThemeDir(themesPath, CURRENT);

    EXPECT_TRUE(QDir(QDir(themesPath).absoluteFilePath(CURRENT)).exists());
    EXPECT_TRUE(QDir(QDir(themesPath).absoluteFilePath(LEGACY)).exists() == false);
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "theme.cfg").toStdString(), std::string("[Appearance]\n"));
}

TEST_F(ThemeMigrationTest, RenamesDirectoryWhenTargetIsFree)
{
    // A customised "Default": stylesheet, zone graphics in a subdirectory and a
    // palette, i.e. everything a user is liable to have personalised.
    writeThemeFile(themesPath, LEGACY, "style.css", "QPushButton { color: red; }");
    writeThemeFile(themesPath, LEGACY, "zones/handzone.png", "handzone-bytes");
    writeThemeFile(themesPath, LEGACY, "zones/tablezone-dark.svg", "tablezone-bytes");
    writeThemeFile(themesPath, LEGACY, "palette-dark.toml", "palette");

    ThemeManager::migrateLegacyThemeDir(themesPath, LEGACY);

    EXPECT_FALSE(QDir(QDir(themesPath).absoluteFilePath(LEGACY)).exists());
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "style.css").toStdString(),
              std::string("QPushButton { color: red; }"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "zones/handzone.png").toStdString(), std::string("handzone-bytes"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "zones/tablezone-dark.svg").toStdString(),
              std::string("tablezone-bytes"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "palette-dark.toml").toStdString(), std::string("palette"));
}

TEST_F(ThemeMigrationTest, RenamesForAnAlreadyMigratedUserWhoseDirectoryWasLeftBehind)
{
    // The settings value was rewritten to "System" on an earlier run, but the
    // customised directory is still called "Default". The customisations are
    // still reachable and must still be carried over.
    writeThemeFile(themesPath, LEGACY, "style.css", "legacy-css");

    ThemeManager::migrateLegacyThemeDir(themesPath, CURRENT);

    EXPECT_FALSE(QDir(QDir(themesPath).absoluteFilePath(LEGACY)).exists());
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "style.css").toStdString(), std::string("legacy-css"));
}

TEST_F(ThemeMigrationTest, MergesIntoAnExistingTargetWithoutOverwriting)
{
    // Post-rename re-customisation: "System" already exists and holds newer
    // files. Those win; the legacy directory only fills the gaps.
    writeThemeFile(themesPath, CURRENT, "style.css", "newer-css");
    writeThemeFile(themesPath, CURRENT, "zones/handzone.png", "newer-handzone");
    writeThemeFile(themesPath, LEGACY, "style.css", "legacy-css");
    writeThemeFile(themesPath, LEGACY, "zones/handzone.png", "legacy-handzone");
    writeThemeFile(themesPath, LEGACY, "zones/stackzone.png", "legacy-stackzone");
    writeThemeFile(themesPath, LEGACY, "palette-light.toml", "legacy-palette");

    ThemeManager::migrateLegacyThemeDir(themesPath, CURRENT);

    EXPECT_FALSE(QDir(QDir(themesPath).absoluteFilePath(LEGACY)).exists());
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "style.css").toStdString(), std::string("newer-css"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "zones/handzone.png").toStdString(), std::string("newer-handzone"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "zones/stackzone.png").toStdString(), std::string("legacy-stackzone"));
    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "palette-light.toml").toStdString(), std::string("legacy-palette"));
}

TEST_F(ThemeMigrationTest, LeavesAnotherThemesDefaultDirectoryAlone)
{
    // A user on an unrelated theme may legitimately have made a "Default"
    // folder of their own; it must not be folded into "System".
    writeThemeFile(themesPath, LEGACY, "style.css", "unrelated");

    ThemeManager::migrateLegacyThemeDir(themesPath, QStringLiteral("Leather"));

    EXPECT_TRUE(QDir(QDir(themesPath).absoluteFilePath(LEGACY)).exists());
    EXPECT_EQ(readThemeFile(themesPath, LEGACY, "style.css").toStdString(), std::string("unrelated"));
    EXPECT_FALSE(QDir(QDir(themesPath).absoluteFilePath(CURRENT)).exists());
}

TEST_F(ThemeMigrationTest, IsIdempotent)
{
    writeThemeFile(themesPath, LEGACY, "style.css", "css");

    ThemeManager::migrateLegacyThemeDir(themesPath, LEGACY);
    ASSERT_EQ(readThemeFile(themesPath, CURRENT, "style.css").toStdString(), std::string("css"));

    ThemeManager::migrateLegacyThemeDir(themesPath, CURRENT);
    ThemeManager::migrateLegacyThemeDir(themesPath, LEGACY);

    EXPECT_EQ(readThemeFile(themesPath, CURRENT, "style.css").toStdString(), std::string("css"));
}

// Every test brings its own main(): GTest is not guaranteed to be found on the build machines, and
// the fallback then links the bare gtest library without gtest_main.
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
