#include "interface/theme_config.h"
#include "interface/theme_icon_badge.h"
#include "interface/theme_icon_fill.h"
#include "interface/theme_icon_sticker.h"
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
#include <gtest/gtest.h>
#include <iterator>

namespace
{
/**
 * @brief Creates the GUI application QPixmap needs, using the offscreen plugin.
 *
 * The theme-migration binary is headless; creating this lazily keeps the other
 * test files free of a display dependency.
 */
void ensureGuiApplication()
{
    static int argc = 1;
    static char name[] = "icon_treatment_preview_test";
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
    return file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();
}

/** @brief Renders a pixmap at device size and tags it with the ratio. */
template <typename Render> QPixmap renderAtDpr(const QSize &logicalSize, qreal dpr, Render &&render)
{
    QPixmap pixmap = render(logicalSize * dpr);
    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}
} // namespace

/**
 * @brief Writes one badge/sticker sheet per shipped palette theme.
 *
 * The treatments are only useful if they read on every palette, so each sheet
 * is rendered from the palette the theme actually ships: all three semantic
 * families, three glyphs each, as line glyphs, stickers and badges, at the two
 * sizes the UI uses, on the window and button surfaces the contrast is measured
 * against. A disabled row closes each family: every treatment falls back to the
 * line glyph there, so an unavailable control still reads as unavailable.
 */
TEST(IconTreatmentPreview, WritesOneSheetPerShippedTheme)
{
    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ensureGuiApplication();
    ASSERT_TRUE(QDir().mkpath(outDir));

    constexpr int MARGIN = 24;
    constexpr int LABEL_WIDTH = 190;
    constexpr int SLOT = 36;
    constexpr int TREATMENT_WIDTH = 120;
    constexpr int ROW_HEIGHT = 56;
    constexpr int PANEL_TITLE = 26;
    constexpr int COLUMN_HEADER = 20;
    constexpr int PANEL_PADDING = 12;
    constexpr qreal DPR = 2.0;
    constexpr int SIZES[] = {16, 24};
    constexpr const char *TREATMENTS[] = {"line", "sticker", "badge"};

    struct Family
    {
        const char *name;
        AppColor::Role role;
        const char *glyphs[3];
    };
    // The hover panel's action grid is the first call site that puts a
    // treatment on an action glyph, so the sheet renders the glyphs that grid
    // actually uses: play/clone/hand (Positive), reduce life and graveyard
    // (Negative), place face down and exile (Informational).
    const QList<Family> families = {
        {"Positive", AppColor::Positive, {"arrow_right", "increment", "arrow_left"}},
        {"Negative", AppColor::Negative, {"arrow_down", "decrement", "remove_row"}},
        {"Informational", AppColor::Informational, {"arrow_right", "view", "arrow_to_top"}},
    };
    const QStringList themes = {
        QStringLiteral("Fabric"), QStringLiteral("Fusion"),       QStringLiteral("Leather"),
        QStringLiteral("Plasma"), QStringLiteral("VelvetMarble"),
    };

    const auto renderLine = [](const QByteArray &svg, const QColor &ink, const QSize &size) {
        QSvgRenderer renderer(resolveSvgColorSentinel(svg, ink.name(QColor::HexRgb).toLatin1()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
        return pixmap;
    };

    const int treatmentCount = static_cast<int>(std::size(TREATMENTS));
    const int panelWidth = LABEL_WIDTH + treatmentCount * TREATMENT_WIDTH + PANEL_PADDING;
    const int panelHeight =
        PANEL_TITLE + COLUMN_HEADER + static_cast<int>(families.size()) * 2 * ROW_HEIGHT + PANEL_PADDING;
    const int logicalWidth = 2 * MARGIN + panelWidth;
    const int logicalHeight = MARGIN + 30 + 2 * (panelHeight + 24) + MARGIN;

    for (const QString &theme : themes) {
        for (const bool dark : {false, true}) {
            const QString file =
                QDir(QStringLiteral(COCKATRICE_SOURCE_DIR))
                    .absoluteFilePath(QStringLiteral("cockatrice/themes/%1/palette-default-%2.toml")
                                          .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));
            const PaletteConfig config = PaletteConfig::fromFile(file);
            ASSERT_TRUE(config.hasPalette()) << theme.toStdString();

            const QPalette palette = config.apply(QPalette());
            const QColor window = palette.color(QPalette::Active, QPalette::Window);
            const QColor button = palette.color(QPalette::Active, QPalette::Button);
            const QColor text = palette.color(QPalette::Active, QPalette::WindowText);
            const QColor disabled = palette.color(QPalette::Disabled, QPalette::WindowText);

            QImage image(QSize(qRound(logicalWidth * DPR), qRound(logicalHeight * DPR)),
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(window);
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.scale(DPR, DPR);

            painter.setPen(text);
            painter.setFont(QFont(QStringLiteral("Sans"), 10, QFont::DemiBold));
            painter.drawText(QPointF(MARGIN, MARGIN),
                             QStringLiteral("%1 — %2 scheme, treatments across glyphs")
                                 .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));

            struct Surface
            {
                const char *name;
                QColor background;
            };
            const QList<Surface> surfaces = {
                {"Window surface", window},
                {"Button surface", button},
            };

            qreal panelTop = MARGIN + 30;
            for (const Surface &surface : surfaces) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(surface.background);
                painter.drawRoundedRect(QRectF(MARGIN, panelTop, panelWidth, panelHeight), 8, 8);

                const qreal contentLeft = MARGIN + PANEL_PADDING;
                painter.setPen(text);
                painter.setFont(QFont(QStringLiteral("Sans"), 9, QFont::DemiBold));
                painter.drawText(QPointF(contentLeft, panelTop + 18), QString::fromLatin1(surface.name));

                painter.setFont(QFont(QStringLiteral("Sans"), 8));
                for (int treatment = 0; treatment < treatmentCount; ++treatment) {
                    const qreal x = contentLeft + LABEL_WIDTH + static_cast<qreal>(treatment) * TREATMENT_WIDTH;
                    painter.drawText(QPointF(x, panelTop + PANEL_TITLE + 14),
                                     QString::fromLatin1(TREATMENTS[treatment]) + QStringLiteral("  16 / 24"));
                }

                qreal y = panelTop + PANEL_TITLE + COLUMN_HEADER;
                for (const Family &family : families) {
                    // The authored role is the tone the filled shape carries; the
                    // treatments apply their own fill rule from it.
                    const QColor tone = config.appColors.value(family.role, defaultAppColor(family.role, dark));
                    ASSERT_TRUE(tone.isValid()) << theme.toStdString() << " " << family.name;

                    for (int state = 0; state < 2; ++state) {
                        const bool isDisabled = state == 1;
                        const QColor ink = isDisabled ? disabled : tone;

                        painter.setPen(text);
                        painter.setFont(QFont(QStringLiteral("Sans"), 8, QFont::DemiBold));
                        painter.drawText(
                            QPointF(contentLeft, y + 14),
                            QStringLiteral("%1%2").arg(QString::fromLatin1(family.name),
                                                       isDisabled ? QStringLiteral(" / disabled") : QString()));
                        painter.setFont(QFont(QStringLiteral("Mono"), 7));
                        painter.drawText(QPointF(contentLeft, y + 28), ink.name(QColor::HexRgb));

                        for (int treatment = 0; treatment < treatmentCount; ++treatment) {
                            const qreal cellX =
                                contentLeft + LABEL_WIDTH + static_cast<qreal>(treatment) * TREATMENT_WIDTH;
                            for (std::size_t sizeIndex = 0; sizeIndex < std::size(SIZES); ++sizeIndex) {
                                const int side = SIZES[sizeIndex];
                                const QSize size(side, side);
                                const qreal rowTop = y + (sizeIndex == 0 ? 4 : 28);
                                for (int glyphIndex = 0; glyphIndex < 3; ++glyphIndex) {
                                    const QByteArray svg = readIconSvg(family.glyphs[glyphIndex]);
                                    ASSERT_FALSE(svg.isEmpty()) << family.glyphs[glyphIndex];
                                    const QPixmap pixmap = renderAtDpr(size, DPR, [&](const QSize &pixelSize) {
                                        if (isDisabled || treatment == 0) {
                                            return renderLine(svg, ink, pixelSize);
                                        }
                                        if (treatment == 1) {
                                            return ThemeIconSticker::renderSticker(svg, ink, pixelSize);
                                        }
                                        return ThemeIconBadge::renderBadge(svg, ink, pixelSize,
                                                                           dark ? ThemeIconFill::Scheme::Dark
                                                                                : ThemeIconFill::Scheme::Light);
                                    });
                                    const qreal x =
                                        cellX + 6 + static_cast<qreal>(glyphIndex) * SLOT + (SLOT - side) / 2.0;
                                    painter.drawPixmap(QPointF(x, rowTop + (SLOT - side) / 2.0), pixmap);
                                }
                            }
                        }
                        y += ROW_HEIGHT;
                    }
                }
                panelTop += panelHeight + 24;
            }

            painter.end();
            const QString path = QDir(outDir).absoluteFilePath(
                QStringLiteral("treatment-preview-%1-%2.png")
                    .arg(theme.toLower(), dark ? QStringLiteral("dark") : QStringLiteral("light")));
            ASSERT_TRUE(image.save(path)) << path.toStdString();
        }
    }
}
