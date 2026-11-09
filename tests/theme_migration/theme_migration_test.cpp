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
#include <libcockatrice/rng/rng_abstract.h>
#include <string>

// Owned by main.cpp in the real application; pixel_map_generator.cpp references
// it, so the test binary needs a definition even though it never renders.
ThemeManager *themeManager = nullptr;

// Referenced by other translation units in the server_remote library.
RNG_Abstract *rng = nullptr;

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

TEST(ThemeAppColorTest, SemanticDefaultsFollowTheScheme)
{
    EXPECT_EQ(defaultAppColor(AppColor::Positive, false), QColor(QStringLiteral("#1a7d3a")));
    EXPECT_EQ(defaultAppColor(AppColor::Positive, true), QColor(QStringLiteral("#6dd58c")));
    EXPECT_EQ(defaultAppColor(AppColor::PositiveStrong, false), QColor(QStringLiteral("#007e28")));
    EXPECT_EQ(defaultAppColor(AppColor::PositiveStrong, true), QColor(QStringLiteral("#32c05c")));
    EXPECT_EQ(defaultAppColor(AppColor::PositiveSoft, false), QColor(QStringLiteral("#4c7659")));
    EXPECT_EQ(defaultAppColor(AppColor::PositiveSoft, true), QColor(QStringLiteral("#8ab196")));
    EXPECT_EQ(defaultAppColor(AppColor::Negative, false), QColor(QStringLiteral("#b3261e")));
    EXPECT_EQ(defaultAppColor(AppColor::Negative, true), QColor(QStringLiteral("#ff8a80")));
    EXPECT_EQ(defaultAppColor(AppColor::NegativeStrong, false), QColor(QStringLiteral("#da0b00")));
    EXPECT_EQ(defaultAppColor(AppColor::NegativeStrong, true), QColor(QStringLiteral("#ff8378")));
    EXPECT_EQ(defaultAppColor(AppColor::NegativeSoft, false), QColor(QStringLiteral("#935f5c")));
    EXPECT_EQ(defaultAppColor(AppColor::NegativeSoft, true), QColor(QStringLiteral("#c79e9b")));
    EXPECT_EQ(defaultAppColor(AppColor::Informational, false), QColor(QStringLiteral("#1c64c4")));
    EXPECT_EQ(defaultAppColor(AppColor::Informational, true), QColor(QStringLiteral("#7aa9f2")));
    EXPECT_EQ(defaultAppColor(AppColor::InformationalStrong, false), QColor(QStringLiteral("#0a4fb5")));
    EXPECT_EQ(defaultAppColor(AppColor::InformationalStrong, true), QColor(QStringLiteral("#69aeff")));
    EXPECT_EQ(defaultAppColor(AppColor::InformationalSoft, false), QColor(QStringLiteral("#526a8a")));
    EXPECT_EQ(defaultAppColor(AppColor::InformationalSoft, true), QColor(QStringLiteral("#9db2cd")));
    EXPECT_EQ(defaultAppColor(AppColor::Hidden, false), QColor(QStringLiteral("#6d28d9")));
    EXPECT_EQ(defaultAppColor(AppColor::Hidden, true), QColor(QStringLiteral("#c6a5fa")));
    EXPECT_EQ(defaultAppColor(AppColor::HiddenStrong, false), QColor(QStringLiteral("#4c0f9e")));
    EXPECT_EQ(defaultAppColor(AppColor::HiddenStrong, true), QColor(QStringLiteral("#c99bff")));
    EXPECT_EQ(defaultAppColor(AppColor::HiddenSoft, false), QColor(QStringLiteral("#6a5a8c")));
    EXPECT_EQ(defaultAppColor(AppColor::HiddenSoft, true), QColor(QStringLiteral("#b3a6c9")));
    EXPECT_EQ(defaultAppColor(AppColor::Marked, false), QColor(QStringLiteral("#9a5b05")));
    EXPECT_EQ(defaultAppColor(AppColor::Marked, true), QColor(QStringLiteral("#e2a63a")));
    EXPECT_EQ(defaultAppColor(AppColor::MarkedStrong, false), QColor(QStringLiteral("#8a4f02")));
    EXPECT_EQ(defaultAppColor(AppColor::MarkedStrong, true), QColor(QStringLiteral("#ffb020")));
    EXPECT_EQ(defaultAppColor(AppColor::MarkedSoft, false), QColor(QStringLiteral("#7b6a4a")));
    EXPECT_EQ(defaultAppColor(AppColor::MarkedSoft, true), QColor(QStringLiteral("#c2ab7d")));
    EXPECT_EQ(defaultAppColor(AppColor::Movement, false), QColor(QStringLiteral("#0e7a72")));
    EXPECT_EQ(defaultAppColor(AppColor::Movement, true), QColor(QStringLiteral("#4cc2b5")));
    EXPECT_EQ(defaultAppColor(AppColor::MovementStrong, false), QColor(QStringLiteral("#096b62")));
    EXPECT_EQ(defaultAppColor(AppColor::MovementStrong, true), QColor(QStringLiteral("#2dd4bf")));
    EXPECT_EQ(defaultAppColor(AppColor::MovementSoft, false), QColor(QStringLiteral("#4a6b67")));
    EXPECT_EQ(defaultAppColor(AppColor::MovementSoft, true), QColor(QStringLiteral("#a3c1bc")));
    // Accent and neutral icon roles have no static default: an older theme
    // written before the roles existed falls back to deriving them from its
    // palette rather than pinning one scheme's ink.
    for (const bool dark : {false, true}) {
        EXPECT_FALSE(defaultAppColor(AppColor::AccentStrong, dark).isValid());
        EXPECT_FALSE(defaultAppColor(AppColor::AccentSoft, dark).isValid());
        EXPECT_FALSE(defaultAppColor(AppColor::Icon, dark).isValid());
        EXPECT_FALSE(defaultAppColor(AppColor::IconStrong, dark).isValid());
        EXPECT_FALSE(defaultAppColor(AppColor::IconSoft, dark).isValid());
    }
}

TEST(ThemeAppColorTest, PaletteConfigRoundTripsSemanticRoles)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    PaletteConfig config;
    const QMap<AppColor::Role, QColor> roles = {
        {AppColor::Icon, QColor(QStringLiteral("#2f59ad"))},
        {AppColor::IconStrong, QColor(QStringLiteral("#213e79"))},
        {AppColor::IconSoft, QColor(QStringLiteral("#687899"))},
        {AppColor::Positive, QColor(QStringLiteral("#1b7f3b"))},
        {AppColor::PositiveStrong, QColor(QStringLiteral("#104a22"))},
        {AppColor::PositiveSoft, QColor(QStringLiteral("#5d7f6f"))},
        {AppColor::Negative, QColor(QStringLiteral("#b3261e"))},
        {AppColor::NegativeStrong, QColor(QStringLiteral("#7c1b15"))},
        {AppColor::NegativeSoft, QColor(QStringLiteral("#9a6c6f"))},
    };
    for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
        config.appColors[it.key()] = it.value();
    }

    const QString path = QDir(dir.path()).absoluteFilePath(QStringLiteral("palette-light.toml"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(config.toToml().toUtf8());
    file.close();

    const PaletteConfig parsed = PaletteConfig::fromFile(path);
    for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
        ASSERT_TRUE(parsed.appColors.contains(it.key()));
        EXPECT_EQ(parsed.appColors.value(it.key()), it.value());
    }
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
    // appears on save, so a fresh profile has no resolved directory. A shipped
    // theme may set ColorScheme = Dark; inheriting it there would force dark
    // mode on every fresh profile regardless of the OS. Only an existing user
    // directory is layered on the shipped theme.
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
