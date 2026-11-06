#include "interface/theme_icon.h"
#include "interface/theme_icon_badge.h"
#include "interface/theme_icon_fill.h"
#include "interface/theme_icon_ink.h"
#include "interface/theme_icon_sticker.h"
#include "interface/theme_manager.h"
#include "libcockatrice/utility/svg_recolor.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QStringList>
#include <QStringLiteral>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <iterator>

namespace
{
/**
 * @brief Creates the GUI application QPixmap needs, using the offscreen plugin.
 *
 * The theme-migration binary is headless; creating this lazily keeps its other
 * tests free of a display dependency.
 */
void ensureGuiApplication()
{
    static int argc = 1;
    static char name[] = "theme_icon_test";
    static char *argv[] = {name, nullptr};
    // Leaked on purpose: a function-local static QApplication is destroyed before Qt's
    // own library statics at exit, and the network module's destructors then run
    // against a torn-down QObject layer (SEGV under AddressSanitizer).
    static QApplication *app = new QApplication(argc, argv);
    Q_UNUSED(app);
}

QString resourcesDir()
{
    return QDir(QStringLiteral(COCKATRICE_SOURCE_DIR)).absoluteFilePath(QStringLiteral("cockatrice/resources"));
}

/** @brief Reads a sentinel SVG from the shipped icon set. */
QByteArray readIconSvg(const char *name)
{
    QFile file(QDir(resourcesDir())
                   .absoluteFilePath(QStringLiteral("icons/") + QString::fromLatin1(name) + QStringLiteral(".svg")));
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

int countOpaquePixels(const QPixmap &pixmap, const QColor &expected)
{
    const QImage image = pixmap.toImage();
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 200 && pixel.red() == expected.red() && pixel.green() == expected.green() &&
                pixel.blue() == expected.blue()) {
                ++count;
            }
        }
    }
    return count;
}

/**
 * @brief The ink the engine resolves a neutral glyph to, for @p palette.
 *
 * Neutral glyphs are not painted with WindowText verbatim: theme_icon_ink
 * borrows accent chroma and holds the result to a contrast floor. Tests that
 * pin the rendered color ask the rule instead of restating it.
 */
QColor expectedNeutralInk(const QPalette &palette)
{
    return ThemeIconInk::glyphInk(palette.color(QPalette::Active, QPalette::WindowText),
                                  palette.color(QPalette::Active, QPalette::Window),
                                  palette.color(QPalette::Active, QPalette::Highlight));
}

/** @brief Opaque pixels that are not the given rim color, i.e. the gradient fill. */ int
countOpaquePixelsOtherThan(const QPixmap &pixmap, const QColor &excluded)
{
    const QImage image = pixmap.toImage();
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 200 && (pixel.red() != excluded.red() || pixel.green() != excluded.green() ||
                                        pixel.blue() != excluded.blue())) {
                ++count;
            }
        }
    }
    return count;
}

QByteArray svgWith(const QByteArray &colorToken)
{
    return QByteArrayLiteral(R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path fill=")") +
           colorToken + QByteArrayLiteral(R"(" d="M2 2h20v20H2z"/></svg>)");
}

/**
 * @brief Restores the global theme search paths and palette after each test.
 *
 * The icon engine reads both process-wide values, so a failing assertion must
 * not leak a temporary theme directory or palette into the next test.
 */
class ThemeIconRenderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        ensureGuiApplication();
        savedSearchPaths = QDir::searchPaths(QStringLiteral("theme"));
        savedPalette = qApp->palette();
        themeManager = new ThemeManager;
        QDir::setSearchPaths(QStringLiteral("theme"), {resourcesDir()});
    }

    void TearDown() override
    {
        delete themeManager;
        themeManager = nullptr;
        QDir::setSearchPaths(QStringLiteral("theme"), savedSearchPaths);
        qApp->setPalette(savedPalette);
    }

    QStringList savedSearchPaths; ///< Search paths captured before the test.
    QPalette savedPalette;        ///< Application palette captured before the test.
};

/** @brief A non-square request must letterbox, not squash, a 24×24 glyph. */
TEST_F(ThemeIconRenderTest, NonSquareRequestPreservesAspectRatio)
{
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/player"));
    ASSERT_FALSE(icon.isNull());

    EXPECT_EQ(icon.actualSize(QSize(20, 15)), QSize(15, 15));

    const QPixmap pixmap = icon.pixmap(QSize(20, 15));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.deviceIndependentSize(), QSizeF(15, 15));
}

/** @brief The engine must follow palette flips, icon modes, and DPR. */
TEST_F(ThemeIconRenderTest, ThemedIconFollowsPaletteModeAndDpr)
{
    QPalette palette = qApp->palette();
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#123456")));
    qApp->setPalette(palette);

    const QIcon icon = loadThemedIcon(QStringLiteral("icons/search"));
    ASSERT_FALSE(icon.isNull());

    const QColor firstInk = expectedNeutralInk(qApp->palette());
    const QPixmap normal = icon.pixmap(QSize(24, 24), 1.0);
    ASSERT_FALSE(normal.isNull());
    EXPECT_EQ(normal.size(), QSize(24, 24));
    EXPECT_DOUBLE_EQ(normal.devicePixelRatio(), 1.0);
    EXPECT_GT(countOpaquePixels(normal, firstInk), 0);

    const QPixmap hidpi = icon.pixmap(QSize(24, 24), 2.0);
    ASSERT_FALSE(hidpi.isNull());
    EXPECT_DOUBLE_EQ(hidpi.devicePixelRatio(), 2.0);
    EXPECT_EQ(hidpi.deviceIndependentSize(), QSizeF(24, 24));

    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#abcdef")));
    qApp->setPalette(palette);
    const QColor secondInk = expectedNeutralInk(qApp->palette());
    EXPECT_NE(firstInk, secondInk) << "the icon did not follow the palette";
    const QPixmap flipped = icon.pixmap(QSize(24, 24), 1.0);
    EXPECT_GT(countOpaquePixels(flipped, secondInk), 0);

    QPalette selectedPalette = qApp->palette();
    selectedPalette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#00ff00")));
    qApp->setPalette(selectedPalette);
    const QPixmap selected = icon.pixmap(QSize(24, 24), 1.0, QIcon::Selected, QIcon::Off);
    EXPECT_GT(countOpaquePixels(selected, QColor(QStringLiteral("#00ff00"))), 0);
}
} // namespace

TEST_F(ThemeIconRenderTest, SentinelUsesPaletteColor)
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(Qt::black));
    palette.setColor(QPalette::WindowText, QColor(Qt::white));
    qApp->setPalette(palette);

    const QPixmap pixmap = loadSentinelSvgPixmap(QStringLiteral("icons/search.svg"));

    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(24, 24));
    EXPECT_GT(countOpaquePixels(pixmap, expectedNeutralInk(qApp->palette())), 0);
}

TEST_F(ThemeIconRenderTest, NonSentinelSvgReturnsNull)
{
    QTemporaryDir themeDir;
    ASSERT_TRUE(themeDir.isValid());
    QFile file(QDir(themeDir.path()).absoluteFilePath(QStringLiteral("plain.svg")));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(svgWith("#000000"));
    file.close();
    QDir::addSearchPath(QStringLiteral("theme"), themeDir.path());

    EXPECT_TRUE(loadSentinelSvgPixmap(QStringLiteral("plain.svg")).isNull());
    EXPECT_TRUE(loadSentinelSvgPixmap(QStringLiteral("icons/search.png")).isNull());
}

TEST_F(ThemeIconRenderTest, LowercaseSentinelIsResolved)
{
    QTemporaryDir themeDir;
    ASSERT_TRUE(themeDir.isValid());
    QFile file(QDir(themeDir.path()).absoluteFilePath(QStringLiteral("lower.svg")));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(svgWith("currentcolor"));
    file.close();

    QDir::addSearchPath(QStringLiteral("theme"), themeDir.path());

    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#123456")));
    qApp->setPalette(palette);

    const QPixmap pixmap = loadSentinelSvgPixmap(QStringLiteral("lower.svg"));

    ASSERT_FALSE(pixmap.isNull());
    EXPECT_GT(countOpaquePixels(pixmap, expectedNeutralInk(qApp->palette())), 0);
}

/**
 * @brief A semantic tint must paint its theme color, not the selection accent.
 *
 * A theme written before Positive/Negative existed has no stored value, so this
 * also covers the fallback that used to return the highlight color.
 */
TEST_F(ThemeIconRenderTest, AppColorTintUsesSemanticColorNotTheAccent)
{
    QPalette palette = qApp->palette();
    const QColor accent(QStringLiteral("#ff00ff"));
    palette.setColor(QPalette::Active, QPalette::Highlight, accent);
    qApp->setPalette(palette);

    const QColor positive = themeManager->appColor(AppColor::Positive);
    const QColor negative = themeManager->appColor(AppColor::Negative);
    ASSERT_TRUE(positive.isValid());
    ASSERT_TRUE(negative.isValid());
    EXPECT_NE(positive, accent);
    EXPECT_NE(negative, accent);

    const QPixmap positivePixmap = loadSentinelSvgPixmap(QStringLiteral("icons/search.svg"), AppColor::Positive);
    ASSERT_FALSE(positivePixmap.isNull());
    EXPECT_GT(countOpaquePixels(positivePixmap, positive), 0);
    EXPECT_EQ(countOpaquePixels(positivePixmap, accent), 0);

    const QPixmap negativePixmap = loadSentinelSvgPixmap(QStringLiteral("icons/search.svg"), AppColor::Negative);
    ASSERT_FALSE(negativePixmap.isNull());
    EXPECT_GT(countOpaquePixels(negativePixmap, negative), 0);
    EXPECT_EQ(countOpaquePixels(negativePixmap, accent), 0);
}

/** @brief A disabled control must not keep signalling its semantic color. */
TEST_F(ThemeIconRenderTest, DisabledAppColorUsesDisabledForeground)
{
    QPalette palette = qApp->palette();
    const QColor disabledText(QStringLiteral("#fedcba"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    qApp->setPalette(palette);

    const IconTint tint(AppColor::Negative);
    EXPECT_EQ(tint.resolve(QIcon::Disabled), disabledText);
    EXPECT_NE(tint.resolve(QIcon::Normal), disabledText);
}

/** @brief Palette tints keep the WindowText -> HighlightedText selected swap. */
TEST_F(ThemeIconRenderTest, SelectedPaletteTintUsesHighlightedText)
{
    QPalette palette = qApp->palette();
    const QColor activeText(QStringLiteral("#111111"));
    const QColor selectedText(QStringLiteral("#222222"));
    palette.setColor(QPalette::Active, QPalette::WindowText, activeText);
    palette.setColor(QPalette::Active, QPalette::HighlightedText, selectedText);
    qApp->setPalette(palette);

    const IconTint tint(QPalette::WindowText);
    // The normal ink is derived from the active text color, but it is icon ink
    // rather than the text color itself.
    EXPECT_NE(tint.resolve(QIcon::Normal), activeText);
    EXPECT_EQ(tint.resolve(QIcon::Normal), expectedNeutralInk(qApp->palette()));
    // A selected row paints on the highlight, so the ink is the palette's
    // highlighted text rather than the icon ink.
    EXPECT_EQ(tint.resolve(QIcon::Selected), selectedText);
}

/** @brief A selected row falls back to HighlightedText when the tint vanishes. */
TEST_F(ThemeIconRenderTest, SelectedAppColorFallsBackWhenItMatchesTheHighlight)
{
    const QColor semantic = themeManager->appColor(AppColor::Positive);
    const QColor selectedText(QStringLiteral("#fedcba"));
    QPalette palette = qApp->palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, semantic);
    palette.setColor(QPalette::Active, QPalette::HighlightedText, selectedText);
    qApp->setPalette(palette);

    EXPECT_EQ(IconTint(AppColor::Positive).resolve(QIcon::Selected), selectedText);
}

/** @brief A selected row keeps a semantic tint that still contrasts enough. */
TEST_F(ThemeIconRenderTest, SelectedAppColorKeepsAContrastingTint)
{
    QPalette palette = qApp->palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(Qt::black));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(Qt::white));
    qApp->setPalette(palette);

    EXPECT_EQ(IconTint(AppColor::Positive).resolve(QIcon::Selected), themeManager->appColor(AppColor::Positive));
}

/**
 * @brief Line glyphs share the filled treatments' saturation floor.
 *
 * The dark-scheme semantic tones are the visible case, but `AccentSoft` is below
 * the floor in every shipped light palette, so the resolve path is exercised
 * without swapping the theme. A line glyph next to a badge or a sticker has to
 * carry the same tone; reverting the floor here would leave it pale beside them.
 */
TEST_F(ThemeIconRenderTest, LineTintAppliesTheFillSaturationFloor)
{
    const QColor raw = themeManager->appColor(AppColor::AccentSoft);
    ASSERT_TRUE(raw.isValid());
    ASSERT_LT(raw.hsvSaturation(), 170);

    const QColor saturated = ThemeIconFill::saturated(raw);
    EXPECT_NE(saturated, raw);
    EXPECT_EQ(IconTint(AppColor::AccentSoft).resolve(QIcon::Normal), saturated);

    const QIcon icon = loadThemedIcon(QStringLiteral("icons/arrow_right"), AppColor::AccentSoft);
    const QPixmap pixmap = icon.pixmap(QSize(24, 24));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_GT(countOpaquePixels(pixmap, saturated), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, raw), 0);
}

/** @brief A badge is a disc with a gradient fill, a dark ring and a contrast glyph. */
TEST_F(ThemeIconRenderTest, BadgeHasDiscGradientAndRing)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    ASSERT_TRUE(tone.isValid());

    const QIcon icon = loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive, IconStyle::Badge);
    const QPixmap pixmap = icon.pixmap(QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(64, 64));

    const QImage image = pixmap.toImage();
    // The disc fills the circle; the square corners stay transparent.
    EXPECT_EQ(image.pixelColor(0, 0).alpha(), 0);
    EXPECT_GT(image.pixelColor(32, 32).alpha(), 200);

    // The ring is the saturated tone darkened, and it must actually be painted.
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconFill::saturated(tone).darker(300)), 0);

    // The fill runs from a lighter top to a darker bottom. Sample left of the
    // check, which now reaches almost to the disc edge.
    const int fillX = 10;
    const QColor top = image.pixelColor(fillX, 16);
    const QColor bottom = image.pixelColor(fillX, 48);
    ASSERT_GT(top.alpha(), 200);
    ASSERT_GT(bottom.alpha(), 200);
    EXPECT_GT(top.lightness(), bottom.lightness());

    // The glyph uses the contrast color chosen for this tone.
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconBadge::glyphColor(tone)), 0);
}

/**
 * @brief The dark-scheme disc carries the same saturated fill as the sticker.
 *
 * The dark tones are tuned for text and read as pale mint or salmon once they
 * fill a disc, so the badge saturates its fill. The dark scheme keeps the
 * darkened ink the badge had before the fill moved to the sticker ramp.
 */
TEST_F(ThemeIconRenderTest, BadgeDarkSchemeDiscUsesTheSaturatedFill)
{
    const QColor tone = defaultAppColor(AppColor::Positive, true);
    ASSERT_TRUE(tone.isValid());
    ASSERT_GE(tone.value(), 200);

    const QPixmap pixmap = ThemeIconBadge::renderBadge(readIconSvg("ready_start"), tone, QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());
    const QImage image = pixmap.toImage();

    // The disc neither keeps the pale raw tone nor the old unsaturating bottom.
    EXPECT_EQ(countOpaquePixels(pixmap, tone), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, tone.darker(104)), 0);
    const QColor top = image.pixelColor(10, 16);
    const QColor bottom = image.pixelColor(10, 48);
    ASSERT_GT(top.alpha(), 200);
    ASSERT_GT(bottom.alpha(), 200);
    // Antialiasing costs a point or two of saturation at the sample points.
    EXPECT_GE(top.hsvSaturation(), 160);
    EXPECT_GE(bottom.hsvSaturation(), 160);
    EXPECT_GT(top.lightness(), bottom.lightness());

    // The dark scheme keeps the darkened ink, and it is actually painted.
    const QColor ink = ThemeIconBadge::glyphColor(tone);
    ASSERT_NE(ink, QColor(Qt::white));
    EXPECT_GT(countOpaquePixels(pixmap, ink), 0);
}

/**
 * @brief The badge ink follows the scheme.
 *
 * Light-scheme tones keep the white glyph of the old art; dark-scheme tones
 * keep the darkened ink, because the shared saturated disc is light enough that
 * white would sit near 2.7:1 at its top. Pinning both cases stops the glyph from
 * silently flipping back to white when the fill or the palette changes.
 */
TEST_F(ThemeIconRenderTest, BadgeInkFollowsTheScheme)
{
    const struct
    {
        AppColor::Role role;
        bool dark;
    } cases[] = {{AppColor::Positive, false},
                 {AppColor::Negative, false},
                 {AppColor::Positive, true},
                 {AppColor::Negative, true}};

    for (const auto &testCase : cases) {
        const QColor tone = defaultAppColor(testCase.role, testCase.dark);
        ASSERT_TRUE(tone.isValid());
        const QColor ink = ThemeIconBadge::glyphColor(tone);
        if (testCase.dark) {
            EXPECT_NE(ink, QColor(Qt::white));
            EXPECT_LT(ink.value(), 100);
        } else {
            EXPECT_EQ(ink, QColor(Qt::white));
        }
    }
}

/** @brief The badge is rendered at the device-pixel size, not upscaled later. */
TEST_F(ThemeIconRenderTest, BadgeHonoursDevicePixelRatio)
{
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive, IconStyle::Badge);
    const QPixmap hidpi = icon.pixmap(QSize(32, 32), 2.0);

    ASSERT_FALSE(hidpi.isNull());
    EXPECT_EQ(hidpi.size(), QSize(64, 64));
    EXPECT_DOUBLE_EQ(hidpi.devicePixelRatio(), 2.0);
    EXPECT_EQ(hidpi.deviceIndependentSize(), QSizeF(32, 32));
}

/**
 * @brief The player list is the one place the badge has to work, at 15px.
 *
 * A 20x15 icon box yields a 15px canvas. The art this replaces kept its check
 * legible there, so the badge must still paint solid glyph ink rather than an
 * antialiased smudge.
 */
TEST_F(ThemeIconRenderTest, BadgeStaysLegibleAtPlayerListSize)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive, IconStyle::Badge);
    const QPixmap pixmap = icon.pixmap(QSize(20, 15));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.deviceIndependentSize(), QSizeF(15, 15));

    // The disc survives: the corners stay transparent and the center is solid.
    const QImage image = pixmap.toImage();
    EXPECT_EQ(image.pixelColor(0, 0).alpha(), 0);
    EXPECT_GT(image.pixelColor(7, 7).alpha(), 200);
    // A 1px ring on a 15px canvas antialiases into the disc and has no exact
    // color, so the glyph is what has to stay legible.
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconBadge::glyphColor(tone)), 8);
}

/** @brief The pixmap API's badge path agrees with the engine's. */
TEST_F(ThemeIconRenderTest, SentinelSvgBadgeMatchesTheEngine)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QPixmap pixmap = loadSentinelSvgPixmap(QStringLiteral("icons/ready_start.svg"), AppColor::Positive,
                                                 QSize(48, 48), IconStyle::Badge);
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(48, 48));
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconFill::saturated(tone).darker(300)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconBadge::glyphColor(tone)), 0);
}

/**
 * @brief Hides Qt's plugin directories while it lives, then restores them.
 *
 * Only the image-format plugins matter here: the platform plugin the fixture
 * needs is already loaded by the time a test runs.
 */
class ScopedPluginPathHiding
{
public:
    ScopedPluginPathHiding() : saved(QCoreApplication::libraryPaths())
    {
        QCoreApplication::setLibraryPaths({});
    }
    ~ScopedPluginPathHiding()
    {
        QCoreApplication::setLibraryPaths(saved);
    }
    ScopedPluginPathHiding(const ScopedPluginPathHiding &) = delete;
    ScopedPluginPathHiding &operator=(const ScopedPluginPathHiding &) = delete;

private:
    QStringList saved;
};

/**
 * @brief Authored SVG art must not depend on Qt's SVG image-format plugin.
 *
 * Loading an `.svg` through QPixmap needs the separate `qsvg` image plugin.
 * Distributions that build Qt with the plugin split out (Debian, and therefore
 * CI) ship it in its own package, and a `--no-install-recommends` install does
 * not have it, so authored theme art rendered blank there while working on a
 * developer machine whose Qt bundles the format. Hiding the plugin directories
 * reproduces that install.
 */
TEST_F(ThemeIconRenderTest, AuthoredArtDoesNotNeedTheSvgImagePlugin)
{
    QTemporaryDir themeDir;
    ASSERT_TRUE(themeDir.isValid());
    QFile file(QDir(themeDir.path()).absoluteFilePath(QStringLiteral("plain.svg")));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(svgWith("#336699"));
    file.close();
    QDir::addSearchPath(QStringLiteral("theme"), themeDir.path());

    // The offscreen platform plugin is already loaded by the fixture, so an
    // empty library path only removes the image-format plugins.
    const ScopedPluginPathHiding hidePlugins;

    const QIcon icon = loadThemedIcon(QStringLiteral("plain"));
    const QPixmap pixmap = icon.pixmap(QSize(32, 32));

    ASSERT_FALSE(pixmap.isNull()) << "authored SVG art went through the QPixmap loader";
    EXPECT_GT(countOpaquePixels(pixmap, QColor(QStringLiteral("#336699"))), 0);
}

/** @brief Badge never paints authored art: the asset keeps its own colors. */
TEST_F(ThemeIconRenderTest, BadgeOnAuthoredArtUsesTheAssetPath)
{
    QTemporaryDir themeDir;
    ASSERT_TRUE(themeDir.isValid());
    QFile file(QDir(themeDir.path()).absoluteFilePath(QStringLiteral("plain.svg")));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(svgWith("#336699"));
    file.close();
    QDir::addSearchPath(QStringLiteral("theme"), themeDir.path());

    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QIcon icon = loadThemedIcon(QStringLiteral("plain"), AppColor::Positive, IconStyle::Badge);
    const QPixmap pixmap = icon.pixmap(QSize(32, 32));
    ASSERT_FALSE(pixmap.isNull());

    // The authored color survives; no disc tone and no ring are painted.
    EXPECT_GT(countOpaquePixels(pixmap, QColor(QStringLiteral("#336699"))), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, tone), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, ThemeIconFill::saturated(tone).darker(300)), 0);
}

/** @brief Disabled keeps the shape language but drops back to the line glyph. */
TEST_F(ThemeIconRenderTest, DisabledBadgeFallsBackToLineGlyph)
{
    QPalette palette = qApp->palette();
    const QColor disabledText(QStringLiteral("#fedcba"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    qApp->setPalette(palette);

    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive, IconStyle::Badge);
    const QPixmap pixmap = icon.pixmap(QSize(64, 64), QIcon::Disabled);
    ASSERT_FALSE(pixmap.isNull());

    // A filled disc at full mass would not read as unavailable, so Disabled
    // drops the badge: no ring, no disc, just the disabled line glyph.
    EXPECT_EQ(countOpaquePixels(pixmap, ThemeIconFill::saturated(tone).darker(300)), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, disabledText.darker(300)), 0);
    EXPECT_EQ(pixmap.toImage().pixelColor(0, 0).alpha(), 0);
    EXPECT_GT(countOpaquePixels(pixmap, disabledText), 0);
}

/**
 * @brief Sticker fills the glyph with the tone and rims it with a dark outline.
 *
 * The old add/remove art was a filled shape with a black outline and no disc, so
 * the sticker must show a solid tone interior, a dark rim, and transparent
 * corners.
 */
TEST_F(ThemeIconRenderTest, StickerPaintsToneFillWithDarkOutline)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/increment"), AppColor::Positive, IconStyle::Sticker);
    const QPixmap pixmap = icon.pixmap(QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(64, 64));

    const QImage image = pixmap.toImage();
    // No disc: the corners stay transparent, unlike the badge.
    EXPECT_EQ(image.pixelColor(0, 0).alpha(), 0);
    EXPECT_EQ(image.pixelColor(63, 63).alpha(), 0);

    // The glyph is filled: the vertical arm carries a gradient, lighter at the
    // top and darker at the bottom, with the rim painted in the darkened tone.
    const QColor top = image.pixelColor(32, 12);
    const QColor bottom = image.pixelColor(32, 52);
    ASSERT_GT(top.alpha(), 200);
    ASSERT_GT(bottom.alpha(), 200);
    EXPECT_GT(top.lightness(), bottom.lightness());
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconSticker::outlineColor(tone)), 20);
}

/**
 * @brief The dark-scheme fill is the branch the treatment exists for.
 *
 * That tone is light and deliberately desaturated, so a plain gradient paints a
 * pale mint shape. The fill has to raise its saturation and pull its value down
 * instead of up.
 */
TEST_F(ThemeIconRenderTest, StickerDarkSchemeFillIsSaturatedAndDarkerThanTheTone)
{
    const QColor tone = defaultAppColor(AppColor::Positive, true);
    ASSERT_TRUE(tone.isValid());
    ASSERT_GE(tone.value(), 200);

    const QPixmap pixmap = ThemeIconSticker::renderSticker(readIconSvg("increment"), tone, QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());
    const QImage image = pixmap.toImage();

    const QColor top = image.pixelColor(32, 14);
    const QColor bottom = image.pixelColor(32, 50);
    ASSERT_GT(top.alpha(), 200);
    ASSERT_GT(bottom.alpha(), 200);
    EXPECT_LT(top.value(), tone.value());
    EXPECT_LT(bottom.value(), tone.value());
    EXPECT_GT(top.lightness(), bottom.lightness());
    EXPECT_GE(top.hsvSaturation(), 170);
}

/** @brief The sticker is rendered at the device-pixel size, not upscaled later. */
TEST_F(ThemeIconRenderTest, StickerHonoursDevicePixelRatio)
{
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/remove_row"), AppColor::Negative, IconStyle::Sticker);
    const QPixmap hidpi = icon.pixmap(QSize(32, 32), 2.0);

    ASSERT_FALSE(hidpi.isNull());
    EXPECT_EQ(hidpi.size(), QSize(64, 64));
    EXPECT_DOUBLE_EQ(hidpi.devicePixelRatio(), 2.0);
    EXPECT_EQ(hidpi.deviceIndependentSize(), QSizeF(32, 32));
}

/**
 * @brief A sticker has to survive the 16px toolbar slot.
 *
 * A 15/16px canvas leaves a three-pixel band. The tone interior must still be
 * solid ink rather than an antialiased smudge, and the icon must not turn into a
 * disc.
 */
TEST_F(ThemeIconRenderTest, StickerStaysLegibleAtToolbarSize)
{
    const QColor tone = themeManager->appColor(AppColor::Negative);
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/remove_row"), AppColor::Negative, IconStyle::Sticker);
    const QPixmap pixmap = icon.pixmap(QSize(16, 16));
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(16, 16));

    const QImage image = pixmap.toImage();
    EXPECT_EQ(image.pixelColor(0, 0).alpha(), 0);
    EXPECT_GT(countOpaquePixelsOtherThan(pixmap, ThemeIconSticker::outlineColor(tone)), 8);
}

/** @brief The pixmap API's sticker path agrees with the engine's. */
TEST_F(ThemeIconRenderTest, SentinelSvgStickerMatchesTheEngine)
{
    const QColor tone = themeManager->appColor(AppColor::Negative);
    const QPixmap pixmap = loadSentinelSvgPixmap(QStringLiteral("icons/remove_row.svg"), AppColor::Negative,
                                                 QSize(48, 48), IconStyle::Sticker);
    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(48, 48));
    EXPECT_GT(countOpaquePixelsOtherThan(pixmap, ThemeIconSticker::outlineColor(tone)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconSticker::outlineColor(tone)), 0);
}

/** @brief Sticker never paints authored art: the asset keeps its own colors. */
TEST_F(ThemeIconRenderTest, StickerOnAuthoredArtUsesTheAssetPath)
{
    QTemporaryDir themeDir;
    ASSERT_TRUE(themeDir.isValid());
    QFile file(QDir(themeDir.path()).absoluteFilePath(QStringLiteral("plain.svg")));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(svgWith("#336699"));
    file.close();
    QDir::addSearchPath(QStringLiteral("theme"), themeDir.path());

    const QColor tone = themeManager->appColor(AppColor::Negative);
    const QIcon icon = loadThemedIcon(QStringLiteral("plain"), AppColor::Negative, IconStyle::Sticker);
    const QPixmap pixmap = icon.pixmap(QSize(32, 32));
    ASSERT_FALSE(pixmap.isNull());

    EXPECT_GT(countOpaquePixels(pixmap, QColor(QStringLiteral("#336699"))), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, tone), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, ThemeIconSticker::outlineColor(tone)), 0);
}

/** @brief Disabled keeps the shape language but drops back to the line glyph. */
TEST_F(ThemeIconRenderTest, DisabledStickerFallsBackToLineGlyph)
{
    QPalette palette = qApp->palette();
    const QColor disabledText(QStringLiteral("#fedcba"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    qApp->setPalette(palette);

    const QColor tone = themeManager->appColor(AppColor::Negative);
    const QIcon icon = loadThemedIcon(QStringLiteral("icons/remove_row"), AppColor::Negative, IconStyle::Sticker);
    const QPixmap pixmap = icon.pixmap(QSize(64, 64), QIcon::Disabled);
    ASSERT_FALSE(pixmap.isNull());

    // No rim in the sticker tone: Disabled drops the treatment and paints the
    // thin disabled line glyph instead.
    EXPECT_EQ(countOpaquePixels(pixmap, ThemeIconSticker::outlineColor(tone)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, disabledText), 0);
}

/** @brief Line stays the default: no disc, no ring, just the recolored glyph. */
TEST_F(ThemeIconRenderTest, LineStyleStillRendersGlyphAlone)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QPixmap pixmap =
        loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive).pixmap(QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());

    EXPECT_EQ(countOpaquePixels(pixmap, ThemeIconFill::saturated(tone).darker(300)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, tone), 0);
}

/**
 * @brief Writes the semantic-tint preview sheet when asked; skipped otherwise.
 *
 * #7775 put the semantic tone back on line glyphs, before any filled treatment
 * existed. The sheet renders the affected glyphs with the raw per-scheme
 * `AppColor` value, which is what that PR paints, at the sizes the UI uses.
 */
TEST(IconTintPreview, WritesPngsWhenRequested)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    ensureGuiApplication();

    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ASSERT_TRUE(QDir().mkpath(outDir));

    struct Glyph
    {
        const char *name;
        AppColor::Role role;
    };
    constexpr Glyph tinted[] = {
        {"ready_start", AppColor::Positive}, {"not_ready_start", AppColor::Negative}, {"increment", AppColor::Positive},
        {"decrement", AppColor::Negative},   {"remove_row", AppColor::Negative},      {"delete", AppColor::Negative},
        {"arrow_right", AppColor::Positive},
    };
    constexpr int SIZES[] = {15, 16, 24, 48};

    const auto renderLine = [](const QByteArray &svg, const QColor &tone, const QSize &size) {
        QSvgRenderer renderer(resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    constexpr int MARGIN = 24;
    constexpr int LABEL_WIDTH = 130;
    constexpr int CELL_WIDTH = 120;
    constexpr int ROW_HEIGHT = 70;
    constexpr qreal DPR = 2.0;

    const int logicalWidth = MARGIN + LABEL_WIDTH + static_cast<int>(std::size(SIZES)) * CELL_WIDTH + MARGIN;
    const int logicalHeight = MARGIN + 24 + 34 + static_cast<int>(std::size(tinted)) * ROW_HEIGHT + 40 + MARGIN;
    const QSize imageSize(qRound(logicalWidth * DPR), qRound(logicalHeight * DPR));

    for (const bool dark : {false, true}) {
        const QColor window(dark ? QStringLiteral("#1e1e1e") : QStringLiteral("#ffffff"));
        const QColor text(dark ? QStringLiteral("#e8e8e8") : QStringLiteral("#1e1e1e"));

        QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(window);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(DPR, DPR);
        painter.setPen(text);
        painter.setFont(QFont(QStringLiteral("Sans"), 9));

        qreal y = MARGIN + 4;
        painter.drawText(QPointF(MARGIN, y), dark ? QStringLiteral("Dark scheme") : QStringLiteral("Light scheme"));

        y += 24;
        painter.drawText(QPointF(MARGIN, y), QStringLiteral("Semantic tints on line glyphs (#7775)"));
        y += 10;

        for (const Glyph &glyph : tinted) {
            y += ROW_HEIGHT;
            const qreal rowTop = y - ROW_HEIGHT;
            QFile file(QDir(resourcesDir())
                           .absoluteFilePath(QStringLiteral("icons/") + QString::fromLatin1(glyph.name) +
                                             QStringLiteral(".svg")));
            ASSERT_TRUE(file.open(QIODevice::ReadOnly)) << glyph.name;
            const QByteArray svg = file.readAll();
            const QColor tone = defaultAppColor(glyph.role, dark);
            painter.drawText(QPointF(MARGIN, rowTop + 16), QString::fromLatin1(glyph.name));
            for (std::size_t i = 0; i < std::size(SIZES); ++i) {
                const int side = SIZES[i];
                const QPixmap pixmap = renderLine(svg, tone, QSize(side, side));
                const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH;
                // Bottom-align the sizes and put the caption below the 48px row.
                painter.drawPixmap(QPointF(x, rowTop + 54 - side), pixmap);
                painter.drawText(QPointF(x, rowTop + 66), QStringLiteral("%1px").arg(side));
            }
        }

        y += 20;
        painter.drawText(QPointF(MARGIN, y),
                         QStringLiteral("Positive: ready and add. Negative: not ready, remove and delete."));

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("tint-preview-dark.png")
                                                                : QStringLiteral("tint-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}

/**
 * @brief Writes the sticker preview sheet when asked; skipped otherwise.
 *
 * The sheet shows each add/remove glyph as line and as sticker at the
 * sizes the UI uses (15px in the player list and menus, 16px in toolbars, 24 and
 * 48 for inspection), with the ready/not-ready badge for context. It lives in the
 * test binary so the colors come from the same place the engine uses.
 */
TEST(IconStickerPreview, WritesPngsWhenRequested)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    ensureGuiApplication();

    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ASSERT_TRUE(QDir().mkpath(outDir));

    struct Glyph
    {
        const char *name;
        AppColor::Role role;
    };
    constexpr Glyph stickered[] = {
        {"increment", AppColor::Positive},
        {"decrement", AppColor::Negative},
        {"remove_row", AppColor::Negative},
        {"delete", AppColor::Negative},
    };
    constexpr int SIZES[] = {15, 16, 24, 48};
    constexpr int CONTEXT_SIZE = 32;

    const auto readSvg = [](const char *name) {
        QFile file(
            QDir(resourcesDir())
                .absoluteFilePath(QStringLiteral("icons/") + QString::fromLatin1(name) + QStringLiteral(".svg")));
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };
    const auto renderLine = [](const QByteArray &svg, const QColor &tone, const QSize &size) {
        QSvgRenderer renderer(resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    constexpr int MARGIN = 24;
    constexpr int LABEL_WIDTH = 110;
    constexpr int CELL_WIDTH = 120;
    constexpr int ROW_HEIGHT = 58;
    constexpr int GLYPH_BLOCK_HEIGHT = 22 + 2 * ROW_HEIGHT;
    constexpr qreal DPR = 2.0;

    const int logicalWidth = MARGIN + LABEL_WIDTH + static_cast<int>(std::size(SIZES)) * CELL_WIDTH + MARGIN;
    const int logicalHeight = MARGIN + 24 + 24 + static_cast<int>(std::size(stickered)) * GLYPH_BLOCK_HEIGHT + 24 + 24 +
                              CONTEXT_SIZE + 46 + CONTEXT_SIZE + 46 + MARGIN;
    const QSize imageSize(qRound(logicalWidth * DPR), qRound(logicalHeight * DPR));

    for (const bool dark : {false, true}) {
        const QColor window(dark ? QStringLiteral("#1e1e1e") : QStringLiteral("#ffffff"));
        const QColor text(dark ? QStringLiteral("#e8e8e8") : QStringLiteral("#1e1e1e"));

        QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(window);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(DPR, DPR);
        painter.setPen(text);
        painter.setFont(QFont(QStringLiteral("Sans"), 9));

        qreal y = MARGIN + 4;
        painter.drawText(QPointF(MARGIN, y), dark ? QStringLiteral("Dark scheme") : QStringLiteral("Light scheme"));

        y += 24;
        painter.drawText(QPointF(MARGIN, y), QStringLiteral("Add / remove (line above, sticker below)"));
        for (const Glyph &glyph : stickered) {
            y += GLYPH_BLOCK_HEIGHT;
            const QByteArray svg = readSvg(glyph.name);
            ASSERT_FALSE(svg.isEmpty()) << glyph.name;
            // The preview must show the line glyph the engine paints, and the
            // engine saturates AppColor tones before drawing them.
            const QColor tone = ThemeIconFill::saturated(defaultAppColor(glyph.role, dark));

            for (int row = 0; row < 2; ++row) {
                const bool sticker = row == 1;
                const qreal rowTop = y - GLYPH_BLOCK_HEIGHT + 22 + row * ROW_HEIGHT;
                painter.drawText(QPointF(MARGIN, rowTop + 16),
                                 (sticker ? QStringLiteral("sticker ") : QStringLiteral("line    ")) +
                                     QString::fromLatin1(glyph.name));
                for (std::size_t i = 0; i < std::size(SIZES); ++i) {
                    const int side = SIZES[i];
                    const QPixmap pixmap = sticker ? ThemeIconSticker::renderSticker(svg, tone, QSize(side, side))
                                                   : renderLine(svg, tone, QSize(side, side));
                    const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH;
                    painter.drawPixmap(QPointF(x, rowTop + 36 - side), pixmap);
                    if (row == 1) {
                        painter.drawText(QPointF(x, rowTop + 54), QStringLiteral("%1px").arg(side));
                    }
                }
            }
        }

        y += 24;
        painter.drawText(QPointF(MARGIN, y),
                         QStringLiteral("Ready / not ready keep the badge (%1px)").arg(CONTEXT_SIZE));
        constexpr Glyph badgeContext[] = {
            {"ready_start", AppColor::Positive},
            {"not_ready_start", AppColor::Negative},
        };
        for (std::size_t i = 0; i < std::size(badgeContext); ++i) {
            const QByteArray svg = readSvg(badgeContext[i].name);
            ASSERT_FALSE(svg.isEmpty()) << badgeContext[i].name;
            const QColor tone = defaultAppColor(badgeContext[i].role, dark);
            const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH;
            painter.drawPixmap(QPointF(x, y + 8),
                               ThemeIconBadge::renderBadge(svg, tone, QSize(CONTEXT_SIZE, CONTEXT_SIZE)));
            painter.drawText(QPointF(x, y + 8 + CONTEXT_SIZE + 12), QString::fromLatin1(badgeContext[i].name));
        }

        y += CONTEXT_SIZE + 46;
        painter.drawText(QPointF(MARGIN, y),
                         QStringLiteral("Line / badge / sticker share one tone (%1px)").arg(CONTEXT_SIZE));
        constexpr const char *FAMILY_LABELS[] = {"line", "badge", "sticker"};
        for (std::size_t i = 0; i < std::size(badgeContext); ++i) {
            const QByteArray svg = readSvg(badgeContext[i].name);
            ASSERT_FALSE(svg.isEmpty()) << badgeContext[i].name;
            const QColor tone = ThemeIconFill::saturated(defaultAppColor(badgeContext[i].role, dark));
            const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH * 1.5;
            for (int j = 0; j < 3; ++j) {
                const QSize side(CONTEXT_SIZE, CONTEXT_SIZE);
                QPixmap pixmap;
                if (j == 0) {
                    pixmap = renderLine(svg, tone, side);
                } else if (j == 1) {
                    pixmap = ThemeIconBadge::renderBadge(svg, tone, side);
                } else {
                    pixmap = ThemeIconSticker::renderSticker(svg, tone, side);
                }
                const qreal px = x + j * (CONTEXT_SIZE + 16);
                painter.drawPixmap(QPointF(px, y + 8), pixmap);
                painter.drawText(QPointF(px, y + 8 + CONTEXT_SIZE + 12), QString::fromLatin1(FAMILY_LABELS[j]));
            }
        }

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("sticker-preview-dark.png")
                                                                : QStringLiteral("sticker-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}

/**
 * @brief Writes the PR preview sheets when asked; skipped otherwise.
 *
 * Two panels per scheme: ready/not-ready as line and badge at the sizes the UI
 * actually uses (15px in the player list, 16px in toolbars, 24 and 48 for
 * inspection). It lives in the test binary so the colors come from the same
 * place the engine uses. The add/remove/delete glyphs and their sticker
 * treatment have their own sheet (IconStickerPreview).
 */
TEST(IconBadgePreview, WritesPngsWhenRequested)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    ensureGuiApplication();

    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ASSERT_TRUE(QDir().mkpath(outDir));

    struct Glyph
    {
        const char *name;
        AppColor::Role role;
    };
    constexpr Glyph badged[] = {
        {"ready_start", AppColor::Positive},
        {"not_ready_start", AppColor::Negative},
    };
    constexpr int SIZES[] = {15, 16, 24, 48};

    const auto readSvg = [](const char *name) {
        QFile file(
            QDir(resourcesDir())
                .absoluteFilePath(QStringLiteral("icons/") + QString::fromLatin1(name) + QStringLiteral(".svg")));
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };
    const auto renderLine = [](const QByteArray &svg, const QColor &tone, const QSize &size) {
        QSvgRenderer renderer(resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    constexpr int MARGIN = 24;
    constexpr int LABEL_WIDTH = 90;
    constexpr int CELL_WIDTH = 120;
    constexpr int ROW_HEIGHT = 58;
    constexpr int GLYPH_BLOCK_HEIGHT = 22 + 2 * ROW_HEIGHT;
    constexpr qreal DPR = 2.0;

    const int logicalWidth = MARGIN + LABEL_WIDTH + static_cast<int>(std::size(SIZES)) * CELL_WIDTH + MARGIN;
    const int logicalHeight = MARGIN + 24 + 24 + static_cast<int>(std::size(badged)) * GLYPH_BLOCK_HEIGHT + 30 + MARGIN;
    const QSize imageSize(qRound(logicalWidth * DPR), qRound(logicalHeight * DPR));

    for (const bool dark : {false, true}) {
        const QColor window(dark ? QStringLiteral("#1e1e1e") : QStringLiteral("#ffffff"));
        const QColor text(dark ? QStringLiteral("#e8e8e8") : QStringLiteral("#1e1e1e"));

        QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(window);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(DPR, DPR);
        painter.setPen(text);
        painter.setFont(QFont(QStringLiteral("Sans"), 9));

        qreal y = MARGIN + 4;
        painter.drawText(QPointF(MARGIN, y), dark ? QStringLiteral("Dark scheme") : QStringLiteral("Light scheme"));

        y += 24;
        painter.drawText(QPointF(MARGIN, y), QStringLiteral("Ready / not ready (line above, badge below)"));
        for (const Glyph &glyph : badged) {
            y += GLYPH_BLOCK_HEIGHT;
            const QByteArray svg = readSvg(glyph.name);
            ASSERT_FALSE(svg.isEmpty()) << glyph.name;
            // The preview must show the line glyph the engine paints, and the
            // engine saturates AppColor tones before drawing them.
            const QColor tone = ThemeIconFill::saturated(defaultAppColor(glyph.role, dark));

            for (int row = 0; row < 2; ++row) {
                const bool badge = row == 1;
                const qreal rowTop = y - GLYPH_BLOCK_HEIGHT + 22 + row * ROW_HEIGHT;
                painter.drawText(QPointF(MARGIN, rowTop + 16),
                                 (badge ? QStringLiteral("badge ") : QStringLiteral("line  ")) +
                                     QString::fromLatin1(glyph.name));
                for (std::size_t i = 0; i < std::size(SIZES); ++i) {
                    const int side = SIZES[i];
                    const QPixmap pixmap = badge ? ThemeIconBadge::renderBadge(svg, tone, QSize(side, side))
                                                 : renderLine(svg, tone, QSize(side, side));
                    const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH;
                    // Align all sizes on a common baseline so the weight
                    // difference between 15px and 48px is visible.
                    painter.drawPixmap(QPointF(x, rowTop + 36 - side), pixmap);
                    if (row == 1) {
                        painter.drawText(QPointF(x, rowTop + 54), QStringLiteral("%1px").arg(side));
                    }
                }
            }
        }

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("badge-preview-dark.png")
                                                                : QStringLiteral("badge-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}

namespace
{
/** @brief Palette roles the ink rule reads, from one shipped theme file. */
struct InkPalette
{
    QColor text;
    QColor window;
    QColor button;
    QColor accent;
};

/**
 * @brief Loads a shipped palette file, or returns a default-constructed palette.
 *
 * The theme files are the only place the shipped values live, so the rule is
 * tested against them rather than against a copy of them.
 */
InkPalette shippedPalette(const QString &theme, bool dark)
{
    const QString file =
        QDir(QStringLiteral(COCKATRICE_SOURCE_DIR))
            .absoluteFilePath(QStringLiteral("cockatrice/themes/%1/palette-default-%2.toml")
                                  .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));
    const PaletteConfig config = PaletteConfig::fromFile(file);
    if (!config.hasPalette()) {
        return {};
    }
    const QPalette palette = config.apply(QPalette());
    return {palette.color(QPalette::Active, QPalette::WindowText), palette.color(QPalette::Active, QPalette::Window),
            palette.color(QPalette::Active, QPalette::Button), palette.color(QPalette::Active, QPalette::Highlight)};
}
} // namespace

TEST(IconInk, GivesNeutralGlyphsAccentChroma)
{
    const InkPalette palette = shippedPalette(QStringLiteral("Fabric"), false);
    ASSERT_TRUE(palette.text.isValid());

    const QColor ink = ThemeIconInk::glyphInk(palette.text, palette.window, palette.accent);
    EXPECT_NE(ink, palette.text) << "a glyph must not be painted with body-text ink verbatim";
    // The borrowed accent is a lighter color than the near-black text ink, so
    // the glyph gets a little lighter as well as more chromatic. What must not
    // change is its legibility, its blue cast, and the fact that it reads as a
    // richer color than plain text.
    EXPECT_GT(ink.hsvSaturation(), palette.text.hsvSaturation()) << "the glyph gained no chroma";
    EXPECT_GT(ink.blue(), ink.red()) << "accent chroma did not reach the glyph";
    EXPECT_GE(ThemeIconInk::contrastRatio(ink, palette.window), ThemeIconInk::labelContrastFloor());
}

TEST(IconInk, MeetsTheLabelFloorOnEveryShippedPalette)
{
    const QString themesRoot =
        QDir(QStringLiteral(COCKATRICE_SOURCE_DIR)).absoluteFilePath(QStringLiteral("cockatrice/themes"));
    const QStringList themes = QDir(themesRoot).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    ASSERT_FALSE(themes.isEmpty());

    int checked = 0;
    for (const QString &theme : themes) {
        for (const bool dark : {false, true}) {
            const InkPalette palette = shippedPalette(theme, dark);
            if (!palette.text.isValid()) {
                continue;
            }
            ++checked;
            const QColor ink = ThemeIconInk::glyphInk(palette.text, palette.window, palette.accent);
            EXPECT_GE(ThemeIconInk::contrastRatio(ink, palette.window), ThemeIconInk::labelContrastFloor())
                << theme.toStdString() << (dark ? " dark" : " light");
        }
    }
    EXPECT_GT(checked, 0) << "no shipped palette was loadable";
}

TEST(IconInk, KeepsADeliberateHueWhenTheTextColorIsWeak)
{
    // A theme whose text color is deliberately low-contrast still gets a glyph
    // that clears the floor, and the hue survives the adjustment.
    const QColor surface(QStringLiteral("#f4f4f4"));
    const QColor weakText(QStringLiteral("#c9c9c9"));
    const QColor ink = ThemeIconInk::glyphInk(weakText, surface, QColor(QStringLiteral("#c9c9c9")));
    EXPECT_GE(ThemeIconInk::contrastRatio(ink, surface), ThemeIconInk::labelContrastFloor());
    EXPECT_LT(ink.value(), weakText.value());
}

/**
 * @brief Writes the glyph-ink comparison sheet when asked; skipped otherwise.
 *
 * One row per accent share, so the trade-off between "still neutral chrome" and
 * "clearly an icon" can be judged against the body text drawn beside the glyph.
 * Lives in the test binary so the colors come from the same rule the engine uses.
 */
TEST(IconInkPreview, WritesPngsWhenRequested)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    ensureGuiApplication();

    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ASSERT_TRUE(QDir().mkpath(outDir));

    constexpr const char *GLYPHS[] = {"search", "info", "settings", "filter", "increment"};
    constexpr int SIZES[] = {15, 16, 24, 48};
    constexpr int LAST_SIZE = SIZES[std::size(SIZES) - 1];
    constexpr int CELL_GAP = 10;

    const auto renderLine = [](const QByteArray &svg, const QColor &tone, const QSize &size) {
        QSvgRenderer renderer(resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    constexpr int MARGIN = 20;
    constexpr int SIZE_LABEL_WIDTH = 46;
    constexpr int BODY_WIDTH = 46;
    constexpr int COLUMN_GAP = 28;
    constexpr int ROW_HEIGHT = 58;
    constexpr qreal DPR = 2.0;

    // The same five glyphs at the sizes the client actually draws them, so the
    // comparison is one the reviewer can read rather than a color swatch.
    const int glyphsWidth =
        static_cast<int>(std::size(GLYPHS)) * LAST_SIZE + (static_cast<int>(std::size(GLYPHS)) - 1) * CELL_GAP;
    const int oldX = MARGIN + SIZE_LABEL_WIDTH + BODY_WIDTH;
    const int newX = oldX + glyphsWidth + COLUMN_GAP;
    const int logicalWidth = newX + glyphsWidth + MARGIN;
    const int blockHeight = 46 + static_cast<int>(std::size(SIZES)) * ROW_HEIGHT + 10;
    const int logicalHeight = MARGIN + 24 + 20 + 26 + 2 * blockHeight + MARGIN;

    for (const bool dark : {false, true}) {
        const InkPalette palette = shippedPalette(QStringLiteral("Fabric"), dark);
        ASSERT_TRUE(palette.text.isValid());

        QImage image(QSize(qRound(logicalWidth * DPR), qRound(logicalHeight * DPR)),
                     QImage::Format_ARGB32_Premultiplied);
        image.fill(palette.window);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(DPR, DPR);

        const QFont bodyFont(QStringLiteral("Sans"), 10);
        const QFont labelFont(QStringLiteral("Sans"), 8);
        const QFont captionFont(QStringLiteral("Sans"), 9, QFont::DemiBold);

        qreal y = MARGIN;
        painter.setPen(palette.text);
        painter.setFont(QFont(QStringLiteral("Sans"), 11, QFont::DemiBold));
        painter.drawText(QPointF(MARGIN, y), (dark ? QStringLiteral("Dark scheme") : QStringLiteral("Light scheme")) +
                                                 QStringLiteral(" — neutral glyph ink"));
        y += 18;
        painter.setFont(labelFont);
        painter.drawText(QPointF(MARGIN, y),
                         QStringLiteral("Body text is the same in both columns; only the glyph ink changes."));
        y += 20;

        // The two colors as filled swatches: at 15-24px the glyph difference is
        // a hue shift, and the hex pair makes it measurable at a glance.
        const QColor windowOld = ThemeIconInk::glyphInk(palette.text, palette.window, palette.accent, 0.0);
        const QColor windowNew = ThemeIconInk::glyphInk(palette.text, palette.window, palette.accent);
        qreal swatchX = MARGIN;
        for (const auto &[ink, label] : {std::pair<QColor, QString>(windowOld, QStringLiteral("before")),
                                         std::pair<QColor, QString>(windowNew, QStringLiteral("after"))}) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(ink);
            painter.drawRoundedRect(QRectF(swatchX, y - 10, 14, 14), 3, 3);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(palette.text);
            painter.drawText(QPointF(swatchX + 20, y), QStringLiteral("%1 %2").arg(label, ink.name(QColor::HexRgb)));
            swatchX += 150;
        }
        y += 24;

        for (const bool onButton : {false, true}) {
            const QColor surface = onButton ? palette.button : palette.window;
            const QColor oldInk = ThemeIconInk::glyphInk(palette.text, surface, palette.accent, 0.0);
            const QColor newInk = ThemeIconInk::glyphInk(palette.text, surface, palette.accent);

            const qreal blockTop = y;
            if (onButton) {
                painter.fillRect(QRectF(0, blockTop, logicalWidth, blockHeight), surface);
            }

            painter.setPen(palette.text);
            painter.setFont(captionFont);
            painter.drawText(QPointF(MARGIN, blockTop + 14),
                             onButton ? QStringLiteral("Button surface") : QStringLiteral("Window surface"));
            painter.setFont(labelFont);
            painter.drawText(QPointF(MARGIN, blockTop + 34), QStringLiteral("size"));
            painter.drawText(QPointF(MARGIN + SIZE_LABEL_WIDTH, blockTop + 34), QStringLiteral("body"));
            painter.drawText(QPointF(oldX, blockTop + 34), QStringLiteral("before: WindowText"));
            painter.drawText(
                QPointF(newX, blockTop + 34),
                QStringLiteral("after: %1% accent ink").arg(qRound(ThemeIconInk::defaultAccentMix() * 100)));

            qreal rowY = blockTop + 46;
            for (const int side : SIZES) {
                const qreal baseline = rowY + ROW_HEIGHT / 2.0 + 3.5;
                painter.setFont(labelFont);
                painter.drawText(QPointF(MARGIN, baseline), QStringLiteral("%1 px").arg(side));
                painter.setFont(bodyFont);
                painter.drawText(QPointF(MARGIN + SIZE_LABEL_WIDTH, baseline), QStringLiteral("Aa"));

                for (std::size_t i = 0; i < std::size(GLYPHS); ++i) {
                    const QByteArray svg = readIconSvg(GLYPHS[i]);
                    ASSERT_FALSE(svg.isEmpty()) << GLYPHS[i];
                    const qreal cellOffset = static_cast<qreal>(i) * (side + CELL_GAP);
                    const qreal glyphY = rowY + (ROW_HEIGHT - side) / 2.0;
                    painter.drawPixmap(QPointF(oldX + cellOffset, glyphY), renderLine(svg, oldInk, QSize(side, side)));
                    painter.drawPixmap(QPointF(newX + cellOffset, glyphY), renderLine(svg, newInk, QSize(side, side)));
                }
                rowY += ROW_HEIGHT;
            }
            y = blockTop + blockHeight;
        }

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("ink-preview-dark.png")
                                                                : QStringLiteral("ink-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}

namespace
{
/** @brief Applies a shipped palette file to the application for a test. */
bool applyShippedPalette(const QString &theme, bool dark)
{
    const QString file =
        QDir(QStringLiteral(COCKATRICE_SOURCE_DIR))
            .absoluteFilePath(QStringLiteral("cockatrice/themes/%1/palette-default-%2.toml")
                                  .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));
    const PaletteConfig config = PaletteConfig::fromFile(file);
    if (!config.hasPalette()) {
        return false;
    }
    qApp->setPalette(config.apply(QPalette()));
    return true;
}

QColor surfaceColor()
{
    return qApp->palette().color(QPalette::Active, QPalette::Window);
}
} // namespace

/**
 * @brief The primary action of a surface takes the accent, not a darker gray.
 *
 * A glyph hierarchy cannot be built out of "dark" and "darker": at any usable
 * contrast a neutral ink is already near the palette's pole, so the top level
 * has to be a different color. The accent is the palette's own answer to "which
 * color means action", so primary uses it instead of inventing a token.
 */
TEST_F(ThemeIconRenderTest, PrimaryEmphasisUsesTheAccent)
{
    ASSERT_TRUE(applyShippedPalette(QStringLiteral("Fabric"), false));

    const QColor secondary = IconTint().resolve(QIcon::Normal);
    const QColor primary = IconTint::primary().resolve(QIcon::Normal);

    EXPECT_NE(primary, secondary);
    EXPECT_GT(primary.blue(), primary.red()) << "the primary glyph did not take the accent's blue";
    EXPECT_GE(ThemeIconInk::contrastRatio(primary, surfaceColor()), ThemeIconInk::primaryContrastFloor());
    EXPECT_GE(ThemeIconInk::contrastRatio(primary, surfaceColor()),
              ThemeIconInk::contrastRatio(secondary, surfaceColor()));
}

/**
 * @brief The primary level out-reads the default one on both schemes.
 *
 * The default ink borrows enough accent to sit off the palette's pole (5.4:1 on
 * light, 6.4:1 on dark), so the 7:1 primary floor is measurably above it in both
 * schemes instead of being lost against a near-black or near-white default.
 * Hue alone would not keep the order; the contrast target does.
 */
TEST_F(ThemeIconRenderTest, PrimaryOutReadsSecondaryOnBothSchemes)
{
    for (const bool dark : {false, true}) {
        ASSERT_TRUE(applyShippedPalette(QStringLiteral("Fabric"), dark));
        const QColor surface = surfaceColor();
        const QColor secondary = IconTint().resolve(QIcon::Normal);
        const QColor primary = IconTint::primary().resolve(QIcon::Normal);
        EXPECT_NE(primary, secondary) << (dark ? "dark" : "light");
        EXPECT_GT(ThemeIconInk::contrastRatio(primary, surface), ThemeIconInk::contrastRatio(secondary, surface))
            << (dark ? "dark" : "light");
        EXPECT_GE(ThemeIconInk::contrastRatio(primary, surface), ThemeIconInk::primaryContrastFloor())
            << (dark ? "dark" : "light");
    }
}

/** @brief A quiet glyph recedes toward its surface without dropping below 3:1. */
TEST_F(ThemeIconRenderTest, QuietEmphasisRecedesButStaysLegible)
{
    ASSERT_TRUE(applyShippedPalette(QStringLiteral("Fabric"), false));

    const QColor secondary = IconTint().resolve(QIcon::Normal);
    const QColor quiet = IconTint::quiet().resolve(QIcon::Normal);

    EXPECT_LT(ThemeIconInk::contrastRatio(quiet, surfaceColor()),
              ThemeIconInk::contrastRatio(secondary, surfaceColor()))
        << "the quiet glyph does not recede";
    EXPECT_GE(ThemeIconInk::contrastRatio(quiet, surfaceColor()), ThemeIconInk::quietContrastFloor());
}

/** @brief Emphasis changes how loud a semantic color is, never which color it is. */
TEST_F(ThemeIconRenderTest, EmphasisKeepsASemanticHue)
{
    ASSERT_TRUE(applyShippedPalette(QStringLiteral("Fabric"), false));

    for (const IconEmphasis emphasis : {IconEmphasis::Primary, IconEmphasis::Secondary, IconEmphasis::Quiet}) {
        const QColor ink = IconTint(AppColor::Positive, emphasis).resolve(QIcon::Normal);
        EXPECT_GT(ink.green(), ink.red()) << static_cast<int>(emphasis);
        EXPECT_GT(ink.green(), ink.blue()) << static_cast<int>(emphasis);
    }
}

/** @brief Every emphasis keeps the selected-row swap to HighlightedText. */
TEST_F(ThemeIconRenderTest, SelectedStillFallsBackToHighlightedText)
{
    QPalette palette = qApp->palette();
    const QColor selectedText(QStringLiteral("#222222"));
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(QStringLiteral("#000000")));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, selectedText);
    qApp->setPalette(palette);

    // Black ink on a black highlight is illegible, so the selection wins for
    // both the default and the quiet level.
    EXPECT_EQ(IconTint().resolve(QIcon::Selected), selectedText);
    EXPECT_EQ(IconTint::quiet().resolve(QIcon::Selected), selectedText);
}

/**
 * @brief Writes the emphasis ladder sheet when asked; skipped otherwise.
 *
 * One strip per scheme: a mock toolbar with body text, a quiet labelling glyph,
 * secondary actions, the primary action, and the primary action on an
 * accent-filled button. The point is the hierarchy at a glance, not the glyph.
 */
TEST_F(ThemeIconRenderTest, EmphasisPreviewWritesPngsWhenRequested)
{
    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ASSERT_TRUE(QDir().mkpath(outDir));

    constexpr int MARGIN = 24;
    constexpr int ROW_HEIGHT = 44;
    constexpr int ICON_SIZE = 20;
    constexpr qreal DPR = 2.0;
    constexpr int WIDTH = 760;

    struct Sample
    {
        const char *glyph;
        const char *label;
        IconTint tint;
    };

    for (const bool dark : {false, true}) {
        const InkPalette palette = shippedPalette(QStringLiteral("Fabric"), dark);
        ASSERT_TRUE(palette.text.isValid());
        ASSERT_TRUE(applyShippedPalette(QStringLiteral("Fabric"), dark));

        const QList<Sample> samples = {
            {nullptr, "body text for comparison", {}},
            {"search", "quiet hint", IconTint::quiet()},
            {"settings", "secondary", IconTint()},
            {"arrow_right", "primary", IconTint::primary()},
            {"increment", "primary, positive", IconTint(AppColor::Positive, IconEmphasis::Primary)},
        };

        const int logicalHeight = MARGIN + 28 + static_cast<int>(samples.size()) * ROW_HEIGHT + ROW_HEIGHT + MARGIN;
        QImage image(QSize(qRound(WIDTH * DPR), qRound(logicalHeight * DPR)), QImage::Format_ARGB32_Premultiplied);
        image.fill(palette.window);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(DPR, DPR);

        qreal y = MARGIN;
        painter.setPen(palette.text);
        painter.setFont(QFont(QStringLiteral("Sans"), 10, QFont::DemiBold));
        painter.drawText(QPointF(MARGIN, y), (dark ? QStringLiteral("Dark scheme") : QStringLiteral("Light scheme")) +
                                                 QStringLiteral(" — emphasis ladder"));
        y += 28;

        for (const Sample &sample : samples) {
            if (sample.glyph != nullptr) {
                const QByteArray svg = readIconSvg(sample.glyph);
                ASSERT_FALSE(svg.isEmpty()) << sample.glyph;
                const QColor ink = sample.tint.resolve(QIcon::Normal);
                QSvgRenderer renderer(resolveSvgColorSentinel(svg, ink.name(QColor::HexRgb).toLatin1()));
                QPixmap glyph(QSize(ICON_SIZE, ICON_SIZE));
                glyph.fill(Qt::transparent);
                QPainter glyphPainter(&glyph);
                glyphPainter.setRenderHint(QPainter::Antialiasing, true);
                renderer.render(&glyphPainter);
                glyphPainter.end();
                painter.drawPixmap(QPointF(MARGIN, y - ICON_SIZE + 4), glyph);
            }

            painter.setPen(palette.text);
            painter.setFont(QFont(QStringLiteral("Sans"), 10));
            painter.drawText(QPointF(MARGIN + ICON_SIZE + 16, y), QString::fromLatin1(sample.label));
            y += ROW_HEIGHT;
        }

        // The primary action on an accent-filled button: the container carries
        // the emphasis, and the glyph has to switch to the on-accent ink.
        const QRectF button(MARGIN, y - 28, 220, 34);
        painter.setBrush(palette.accent);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(button, 6, 6);
        painter.setPen(palette.text);
        painter.setFont(QFont(QStringLiteral("Sans"), 10));
        painter.drawText(QPointF(MARGIN + 12, y - 6), QStringLiteral("primary button: accent fill + on-accent glyph"));
        painter.end();

        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("emphasis-preview-dark.png")
                                                                : QStringLiteral("emphasis-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}
