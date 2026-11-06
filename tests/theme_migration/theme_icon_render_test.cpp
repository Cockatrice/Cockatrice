#include "interface/theme_icon.h"
#include "interface/theme_icon_badge.h"
#include "interface/theme_manager.h"
#include "libcockatrice/utility/svg_recolor.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
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

    const QPixmap normal = icon.pixmap(QSize(24, 24), 1.0);
    ASSERT_FALSE(normal.isNull());
    EXPECT_EQ(normal.size(), QSize(24, 24));
    EXPECT_DOUBLE_EQ(normal.devicePixelRatio(), 1.0);
    EXPECT_GT(countOpaquePixels(normal, QColor(QStringLiteral("#123456"))), 0);

    const QPixmap hidpi = icon.pixmap(QSize(24, 24), 2.0);
    ASSERT_FALSE(hidpi.isNull());
    EXPECT_DOUBLE_EQ(hidpi.devicePixelRatio(), 2.0);
    EXPECT_EQ(hidpi.deviceIndependentSize(), QSizeF(24, 24));

    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#abcdef")));
    qApp->setPalette(palette);
    const QPixmap flipped = icon.pixmap(QSize(24, 24), 1.0);
    EXPECT_GT(countOpaquePixels(flipped, QColor(QStringLiteral("#abcdef"))), 0);

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
    palette.setColor(QPalette::WindowText, QColor(Qt::white));
    qApp->setPalette(palette);

    const QPixmap pixmap = loadSentinelSvgPixmap(QStringLiteral("icons/search.svg"));

    ASSERT_FALSE(pixmap.isNull());
    EXPECT_EQ(pixmap.size(), QSize(24, 24));
    EXPECT_GT(countOpaquePixels(pixmap, Qt::white), 0);
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
    EXPECT_GT(countOpaquePixels(pixmap, QColor(QStringLiteral("#123456"))), 0);
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
    EXPECT_EQ(tint.resolve(QIcon::Normal), activeText);
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

    // The ring is the tone darkened, and it must actually be painted.
    EXPECT_GT(countOpaquePixels(pixmap, tone.darker(300)), 0);

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
    EXPECT_GT(countOpaquePixels(pixmap, tone.darker(300)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, ThemeIconBadge::glyphColor(tone)), 0);
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
    EXPECT_EQ(countOpaquePixels(pixmap, tone.darker(300)), 0);
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
    EXPECT_EQ(countOpaquePixels(pixmap, tone.darker(300)), 0);
    EXPECT_EQ(countOpaquePixels(pixmap, disabledText.darker(300)), 0);
    EXPECT_EQ(pixmap.toImage().pixelColor(0, 0).alpha(), 0);
    EXPECT_GT(countOpaquePixels(pixmap, disabledText), 0);
}

/** @brief Line stays the default: no disc, no ring, just the recolored glyph. */
TEST_F(ThemeIconRenderTest, LineStyleStillRendersGlyphAlone)
{
    const QColor tone = themeManager->appColor(AppColor::Positive);
    const QPixmap pixmap =
        loadThemedIcon(QStringLiteral("icons/ready_start"), AppColor::Positive).pixmap(QSize(64, 64));
    ASSERT_FALSE(pixmap.isNull());

    EXPECT_EQ(countOpaquePixels(pixmap, tone.darker(300)), 0);
    EXPECT_GT(countOpaquePixels(pixmap, tone), 0);
}

/**
 * @brief Writes the PR preview sheets when asked; skipped otherwise.
 *
 * Two panels per scheme: ready/not-ready as line and badge at the sizes the UI
 * actually uses (15px in the player list, 16px in toolbars, 24 and 48 for
 * inspection), and the add/remove/delete glyphs, which stay line because the
 * old art was a filled glyph and not a disc. It lives in the test binary so the
 * colors come from the same place the engine uses.
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
    constexpr Glyph unchanged[] = {
        {"increment", AppColor::Positive},
        {"decrement", AppColor::Negative},
        {"remove_row", AppColor::Negative},
        {"delete", AppColor::Negative},
    };
    constexpr int SIZES[] = {15, 16, 24, 48};
    constexpr int LINE_GLYPH_SIZE = 32;

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
    const int logicalHeight = MARGIN + 24 + 24 + static_cast<int>(std::size(badged)) * GLYPH_BLOCK_HEIGHT + 24 + 24 +
                              LINE_GLYPH_SIZE + 30 + MARGIN;
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
            const QColor tone = defaultAppColor(glyph.role, dark);

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

        y += 24;
        painter.drawText(QPointF(MARGIN, y),
                         QStringLiteral("Add / remove / delete (unchanged line glyphs, %1px)").arg(LINE_GLYPH_SIZE));
        for (std::size_t i = 0; i < std::size(unchanged); ++i) {
            const QByteArray svg = readSvg(unchanged[i].name);
            ASSERT_FALSE(svg.isEmpty()) << unchanged[i].name;
            const QColor tone = defaultAppColor(unchanged[i].role, dark);
            const qreal x = MARGIN + LABEL_WIDTH + static_cast<qreal>(i) * CELL_WIDTH;
            painter.drawPixmap(QPointF(x, y + 8), renderLine(svg, tone, QSize(LINE_GLYPH_SIZE, LINE_GLYPH_SIZE)));
            painter.drawText(QPointF(x, y + 8 + LINE_GLYPH_SIZE + 12), QString::fromLatin1(unchanged[i].name));
        }

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(dark ? QStringLiteral("badge-preview-dark.png")
                                                                : QStringLiteral("badge-preview-light.png"));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}
