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
    // Base of the per-user theme tree (<tmp>/themes).
    QString themesPath;
    // Base of the shipped/install theme tree (<tmp>/system), deliberately
    // separate from themesPath so a fallback cannot pass by reading a user file.
    QString systemThemesPath;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        themesPath = QDir(tmp.path()).absoluteFilePath("themes");
        systemThemesPath = QDir(tmp.path()).absoluteFilePath("system");
        ASSERT_TRUE(QDir().mkpath(themesPath));
        ASSERT_TRUE(QDir().mkpath(systemThemesPath));
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

TEST_F(ThemeMigrationTest, ThemeConfigFallsBackToTheShippedThemeDir)
{
    // A user theme directory only holds what the user (or a migration) wrote.
    // Reading theme.cfg from it alone would shadow the shipped theme and leave
    // the board without its style and colour scheme.
    const QString userDir = QDir(themesPath).absoluteFilePath("Leather");
    ASSERT_TRUE(QDir().mkpath(userDir));
    writeThemeFile(themesPath, "Leather", "fonts.toml", "[Fonts]\nCardTextMaxSize = 14\n");
    writeThemeFile(systemThemesPath, "Leather", "theme.cfg",
                   "[Appearance]\nColorScheme = Dark\n\n[Style]\nName = Leather\n");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Leather");
    const ThemeConfig cfg = ThemeManager::loadDefaultThemeConfig(userDir, systemDir);

    EXPECT_EQ(cfg.colorScheme.toStdString(), std::string("Dark"));
    EXPECT_EQ(cfg.styleName.toStdString(), std::string("Leather"));
}

TEST_F(ThemeMigrationTest, UserThemeConfigWinsOverTheShippedOne)
{
    writeThemeFile(themesPath, "Leather", "theme.cfg", "[Appearance]\nColorScheme = Light\n\n[Style]\nName = Custom\n");
    writeThemeFile(systemThemesPath, "Leather", "theme.cfg",
                   "[Appearance]\nColorScheme = Dark\n\n[Style]\nName = Leather\n");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Leather");
    const ThemeConfig cfg =
        ThemeManager::loadDefaultThemeConfig(QDir(themesPath).absoluteFilePath("Leather"), systemDir);

    EXPECT_EQ(cfg.colorScheme.toStdString(), std::string("Light"));
    EXPECT_EQ(cfg.styleName.toStdString(), std::string("Custom"));
}

TEST_F(ThemeMigrationTest, ColorSchemeFollowsTheOsWhenTheResolvedThemeDirIsMissing)
{
    // getAvailableThemes() maps System/Fusion to a user directory that only
    // appears on save, so a fresh profile has no resolved directory. Fusion
    // ships ColorScheme = Dark; inheriting it there would force dark mode on
    // every fresh profile regardless of the OS. Only an existing user directory
    // is layered on the shipped theme.
    const QString userDir = QDir(themesPath).absoluteFilePath("Fusion");
    ASSERT_FALSE(QFileInfo::exists(userDir));
    writeThemeFile(systemThemesPath, "Fusion", "theme.cfg",
                   "[Appearance]\nColorScheme = Dark\n\n[Style]\nName = Fusion\n");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Fusion");
    const ThemeConfig cfg = ThemeManager::loadColorSchemeConfig(userDir, systemDir);

    // Empty means isDarkMode() falls through to the OS color scheme.
    EXPECT_TRUE(cfg.colorScheme.isEmpty());
}

TEST_F(ThemeMigrationTest, ColorSchemeInheritsTheShippedSchemeForAnExistingUserDir)
{
    // The migration seeds an existing user directory with fonts.toml only. Such
    // a directory still belongs to the shipped theme and must keep its scheme
    // instead of silently flipping to the OS one.
    const QString userDir = QDir(themesPath).absoluteFilePath("Fusion");
    ASSERT_TRUE(QDir().mkpath(userDir));
    writeThemeFile(themesPath, "Fusion", "fonts.toml", "[Fonts]\nCardTextMaxSize = 14\n");
    writeThemeFile(systemThemesPath, "Fusion", "theme.cfg",
                   "[Appearance]\nColorScheme = Dark\n\n[Style]\nName = Fusion\n");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Fusion");
    const ThemeConfig cfg = ThemeManager::loadColorSchemeConfig(userDir, systemDir);

    EXPECT_EQ(cfg.colorScheme.toStdString(), std::string("Dark"));
}

TEST_F(ThemeMigrationTest, DefaultThemeConfigStillCarriesTheShippedStyleWithoutAUserDir)
{
    // themeChangedSlot()/previewPalette() use the unconditional fallback: a
    // stock theme with no user directory must still get its shipped styleName,
    // even though its colour scheme follows the OS.
    const QString userDir = QDir(themesPath).absoluteFilePath("Fusion");
    ASSERT_FALSE(QFileInfo::exists(userDir));
    writeThemeFile(systemThemesPath, "Fusion", "theme.cfg",
                   "[Appearance]\nColorScheme = Dark\n\n[Style]\nName = Fusion\n");

    const ThemeConfig cfg =
        ThemeManager::loadDefaultThemeConfig(userDir, QDir(systemThemesPath).absoluteFilePath("Fusion"));

    EXPECT_EQ(cfg.styleName.toStdString(), std::string("Fusion"));
}

TEST_F(ThemeMigrationTest, StyleSheetFallsBackToTheShippedThemeDir)
{
    const QString userDir = QDir(themesPath).absoluteFilePath("Leather");
    ASSERT_TRUE(QDir().mkpath(userDir));
    writeThemeFile(themesPath, "Leather", "fonts.toml", "[Fonts]\nCardTextMaxSize = 14\n");
    writeThemeFile(systemThemesPath, "Leather", "style.css", "shipped-css");
    writeThemeFile(systemThemesPath, "Leather", "style-dark.css", "shipped-dark-css");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Leather");

    // The scheme-qualified shipped stylesheet wins, else the plain one.
    EXPECT_EQ(ThemeManager::themeStyleSheetPath(userDir, systemDir, true).toStdString(),
              QDir(systemDir).absoluteFilePath("style-dark.css").toStdString());
    EXPECT_EQ(ThemeManager::themeStyleSheetPath(userDir, systemDir, false).toStdString(),
              QDir(systemDir).absoluteFilePath("style.css").toStdString());
}

TEST_F(ThemeMigrationTest, StyleSheetPrefersTheUserCopy)
{
    const QString userDir = QDir(themesPath).absoluteFilePath("Leather");
    writeThemeFile(themesPath, "Leather", "style.css", "user-css");
    writeThemeFile(systemThemesPath, "Leather", "style-dark.css", "shipped-dark-css");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Leather");

    // A user-authored stylesheet is never mixed with the shipped scheme variant.
    EXPECT_EQ(ThemeManager::themeStyleSheetPath(userDir, systemDir, true).toStdString(),
              QDir(userDir).absoluteFilePath("style.css").toStdString());
}

TEST_F(ThemeMigrationTest, StyleSheetIsEmptyWhenNeitherDirectoryHasOne)
{
    const QString userDir = QDir(themesPath).absoluteFilePath("Leather");
    ASSERT_TRUE(QDir().mkpath(userDir));
    writeThemeFile(themesPath, "Leather", "fonts.toml", "[Fonts]\nCardTextMaxSize = 14\n");
    // Only the opposite scheme's stylesheet is shipped, so no fallback matches.
    writeThemeFile(systemThemesPath, "Leather", "style-dark.css", "shipped-dark-css");

    const QString systemDir = QDir(systemThemesPath).absoluteFilePath("Leather");

    EXPECT_TRUE(ThemeManager::themeStyleSheetPath(userDir, systemDir, false).isEmpty());
}

// Every test brings its own main(): GTest is not guaranteed to be found on the build machines, and
// the fallback then links the bare gtest library without gtest_main.
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
