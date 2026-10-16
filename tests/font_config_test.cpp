#include "interface/theme_config.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QIODevice>
#include <QString>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTextStream>
#include <gtest/gtest.h>
#include <string>

namespace
{
// Writes content to a file inside tmp, creating parents as needed.
QString writeTempFile(QTemporaryDir &tmp, const QString &relativePath, const QString &content)
{
    const QString filePath = QDir(tmp.path()).absoluteFilePath(relativePath);
    EXPECT_TRUE(QDir().mkpath(QFileInfo(filePath).absolutePath()));
    QFile file(filePath);
    EXPECT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << content;
    file.close();
    return filePath;
}

QFont makeFont(const QString &family, int pointSize)
{
    QFont font(family);
    font.setPointSize(pointSize);
    return font;
}
} // namespace

TEST(FontConfigTest, FileNameIsFontsToml)
{
    EXPECT_EQ(FontConfig::fileName(), QStringLiteral("fonts.toml"));
}

TEST(FontConfigTest, EmptyByDefault)
{
    FontConfig cfg;
    EXPECT_FALSE(cfg.hasFonts());
    EXPECT_TRUE(cfg.toToml().isEmpty());
    EXPECT_EQ(cfg, FontConfig());
    EXPECT_EQ(cfg, FontConfig::fromFile("/nonexistent/path/fonts.toml"));
    EXPECT_EQ(cfg, FontConfig::fromThemeDir(QString()));
}

TEST(FontConfigTest, RoundTripPreservesFamilyAndSize)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    FontConfig cfg;
    cfg.fonts[AppFont::InterfaceDefault] = makeFont("Inter", 10);
    cfg.fonts[AppFont::ChatLog] = makeFont("JetBrains Mono", 9);
    cfg.fonts[AppFont::CardText] = makeFont("Noto Serif", 11);
    cfg.fonts[AppFont::DeckList] = makeFont("Roboto", 12);
    ASSERT_TRUE(cfg.hasFonts());

    const QString filePath = writeTempFile(tmp, "fonts.toml", cfg.toToml());
    const FontConfig loaded = FontConfig::fromFile(filePath);

    EXPECT_EQ(loaded, cfg);
    EXPECT_EQ(loaded.fonts[AppFont::InterfaceDefault].family(), QStringLiteral("Inter"));
    EXPECT_EQ(loaded.fonts[AppFont::InterfaceDefault].pointSize(), 10);
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog].family(), QStringLiteral("JetBrains Mono"));
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog].pointSize(), 9);
    EXPECT_EQ(loaded.fonts[AppFont::CardText].family(), QStringLiteral("Noto Serif"));
    EXPECT_EQ(loaded.fonts[AppFont::CardText].pointSize(), 11);
    EXPECT_EQ(loaded.fonts[AppFont::DeckList].family(), QStringLiteral("Roboto"));
    EXPECT_EQ(loaded.fonts[AppFont::DeckList].pointSize(), 12);
}

TEST(FontConfigTest, RoundTripStripsWeightAndItalic)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    QFont styled("Inter");
    styled.setPointSize(10);
    styled.setBold(true);
    styled.setItalic(true);

    FontConfig cfg;
    cfg.fonts[AppFont::ChatLog] = styled;

    const QString filePath = writeTempFile(tmp, "fonts.toml", cfg.toToml());
    const FontConfig loaded = FontConfig::fromFile(filePath);

    ASSERT_TRUE(loaded.fonts.contains(AppFont::ChatLog));
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog].family(), QStringLiteral("Inter"));
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog].pointSize(), 10);
    EXPECT_FALSE(loaded.fonts[AppFont::ChatLog].bold());
    EXPECT_FALSE(loaded.fonts[AppFont::ChatLog].italic());
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog], makeFont("Inter", 10));
}

TEST(FontConfigTest, ToTomlFormat)
{
    FontConfig cfg;
    cfg.fonts[AppFont::InterfaceDefault] = makeFont("Inter", 10);

    const QString toml = cfg.toToml();
    EXPECT_TRUE(toml.contains(QStringLiteral("[Fonts]")));
    EXPECT_TRUE(toml.contains(QStringLiteral("InterfaceDefault")));
    EXPECT_TRUE(toml.contains(QStringLiteral("\"Inter, 10\"")));
}

TEST(FontConfigTest, MalformedLinesAreSkipped)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    const QString filePath = writeTempFile(tmp, "fonts.toml",
                                           "# a comment\n"
                                           "; another comment\n"
                                           "\n"
                                           "StrayBeforeSection = \"Inter, 10\"\n"
                                           "\n"
                                           "[Other]\n"
                                           "ChatLog = \"ShouldBeIgnored, 10\"\n"
                                           "\n"
                                           "[Fonts]\n"
                                           "InterfaceDefault = \"Inter, 10\"\n"
                                           "ChatLog = \"JetBrains Mono\", 9 # inline comment\n"
                                           "CardText = Noto Serif, 11\n"
                                           "AppFont::DeckList = \"Roboto, 12\"\n"
                                           "UnknownRole = \"Inter, 10\"\n"
                                           "QPalette::Window = \"Inter, 10\"\n"
                                           "NoComma = \"Inter\"\n"
                                           "EmptyFamily = \", 10\"\n"
                                           "ZeroSize = \"Inter, 0\"\n"
                                           "NegativeSize = \"Inter, -3\"\n"
                                           "NotANumber = \"Inter, big\"\n"
                                           "NotAnEntry\n");

    const FontConfig loaded = FontConfig::fromFile(filePath);

    ASSERT_TRUE(loaded.hasFonts());
    EXPECT_EQ(loaded.fonts.size(), 4);
    EXPECT_EQ(loaded.fonts[AppFont::InterfaceDefault], makeFont("Inter", 10));
    EXPECT_EQ(loaded.fonts[AppFont::ChatLog], makeFont("JetBrains Mono", 9));
    EXPECT_EQ(loaded.fonts[AppFont::CardText], makeFont("Noto Serif", 11));
    EXPECT_EQ(loaded.fonts[AppFont::DeckList], makeFont("Roboto", 12));
}

TEST(FontConfigTest, FromThemeDirReadsFontsToml)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    const QString themeDir = QDir(tmp.path()).absoluteFilePath("MyTheme");
    ASSERT_TRUE(QDir().mkpath(themeDir));

    FontConfig cfg;
    cfg.fonts[AppFont::CardText] = makeFont("Noto Serif", 11);
    writeTempFile(tmp, "MyTheme/fonts.toml", cfg.toToml());

    const FontConfig loaded = FontConfig::fromThemeDir(themeDir);
    EXPECT_EQ(loaded, cfg);

    // A theme directory without fonts.toml loads as empty.
    const QString bareDir = QDir(tmp.path()).absoluteFilePath("BareTheme");
    ASSERT_TRUE(QDir().mkpath(bareDir));
    EXPECT_FALSE(FontConfig::fromThemeDir(bareDir).hasFonts());
}

TEST(FontConfigTest, InequalityComparesRoles)
{
    FontConfig left;
    left.fonts[AppFont::InterfaceDefault] = makeFont("Inter", 10);

    FontConfig right = left;
    EXPECT_EQ(left, right);
    EXPECT_FALSE(left != right);

    right.fonts[AppFont::InterfaceDefault] = makeFont("Inter", 11);
    EXPECT_NE(left, right);

    right = left;
    right.fonts[AppFont::ChatLog] = makeFont("Inter", 10);
    EXPECT_NE(left, right);
}

TEST(FontConfigTest, CardTextMinRoundTrip)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    FontConfig cfg;
    cfg.fonts[AppFont::CardText] = makeFont("Noto Serif", 14);
    cfg.cardTextMinSize = 3;
    ASSERT_TRUE(cfg.hasFonts());

    const QString filePath = writeTempFile(tmp, "fonts.toml", cfg.toToml());
    const FontConfig loaded = FontConfig::fromFile(filePath);

    EXPECT_EQ(loaded, cfg);
    EXPECT_EQ(loaded.fonts[AppFont::CardText].pointSize(), 14);
    EXPECT_EQ(loaded.cardTextMinSize, 3);
}

TEST(FontConfigTest, CardTextMinDefaultIsOmitted)
{
    FontConfig cfg;
    cfg.fonts[AppFont::CardText] = makeFont("Noto Serif", 12);
    EXPECT_EQ(cfg.cardTextMinSize, 1);

    const QString toml = cfg.toToml();
    EXPECT_TRUE(toml.contains(QStringLiteral("CardText")));
    EXPECT_FALSE(toml.contains(QStringLiteral("CardTextMinSize")));

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    const QString filePath = writeTempFile(tmp, "fonts.toml", toml);
    EXPECT_EQ(FontConfig::fromFile(filePath).cardTextMinSize, 1);
}

TEST(FontConfigTest, MalformedMinIsSkipped)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    const QString filePath = writeTempFile(tmp, "fonts.toml",
                                           "[Fonts]\n"
                                           "CardText = \"Noto Serif, 14\"\n"
                                           "CardTextMinSize = big\n");

    // Each malformed value parses independently, so rewrite the file per case.
    const QStringList badValues = {"big", "0", "-3", "", "Inter, 10"};
    for (const QString &bad : badValues) {
        const QString badPath = writeTempFile(
            tmp, "bad.toml", QString("[Fonts]\nCardText = \"Noto Serif, 14\"\nCardTextMinSize = %1\n").arg(bad));
        const FontConfig loaded = FontConfig::fromFile(badPath);
        EXPECT_EQ(loaded.cardTextMinSize, 1) << "bad value: " << bad.toStdString();
        ASSERT_TRUE(loaded.fonts.contains(AppFont::CardText));
        EXPECT_EQ(loaded.fonts[AppFont::CardText].pointSize(), 14);
    }

    // The valid file still loads.
    EXPECT_EQ(FontConfig::fromFile(filePath).cardTextMinSize, 1);
}

TEST(FontConfigTest, InequalityComparesMinSize)
{
    FontConfig left;
    left.fonts[AppFont::CardText] = makeFont("Noto Serif", 12);
    left.cardTextMinSize = 1;

    FontConfig right = left;
    right.cardTextMinSize = 2;
    EXPECT_NE(left, right);
}

// Every test brings its own main(): GTest is not guaranteed to be found on the build machines, and
// the fallback then links the bare gtest library without gtest_main.
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
