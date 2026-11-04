#include "interface/theme_icon.h"
#include "interface/theme_manager.h"

#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPalette>
#include <QPixmap>
#include <QSize>
#include <QStringList>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <gtest/gtest.h>

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
