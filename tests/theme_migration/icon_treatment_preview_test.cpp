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
#include <QFontMetrics>
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
        {"Hidden", AppColor::Hidden, {"view", "arrow_right", "swap"}},
        {"Marked", AppColor::Marked, {"pencil", "share", "pin"}},
        {"Movement", AppColor::Movement, {"arrow_to_top", "arrow_to_bottom", "arrow_down"}},
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

namespace
{
/** @brief One action-grid cell, with the role the panel hands the icon engine. */
struct PreviewAction
{
    QString label;
    const char *glyph;
    AppColor::Role role;
};

/** @brief One titled section of a hover panel page. */
struct PreviewSection
{
    QString title;
    QList<PreviewAction> actions;
};

/** @brief One hover panel page, as the client builds it. */
struct PreviewPage
{
    QString title;
    QList<PreviewSection> sections;
};

/** @brief Renders a sentinel SVG in one ink at the requested pixel size. */
QPixmap renderInkGlyph(const QByteArray &svg, const QColor &ink, const QSize &size)
{
    QSvgRenderer renderer(resolveSvgColorSentinel(svg, ink.name(QColor::HexRgb).toLatin1()));
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter);
    return pixmap;
}
} // namespace

/**
 * @brief Writes the hover panel's action grids the way the client paints them.
 *
 * The treatment sheets show glyphs in isolation; this renders the real layout —
 * section headers and two-column cells with the glyph above its label — so the
 * emphasis split (quiet tabs, primary grid) and the semantic map can be judged
 * as the set the player actually sees. The roles are the ones the panel passes
 * to the icon engine, already resolved to the emphasis level it requests, and a
 * role the theme does not author resolves through the same scheme default the
 * client paints.
 */
TEST(IconTreatmentPreview, HoverPanelGridWritesPngsWhenRequested)
{
    const QString outDir = qEnvironmentVariable("COCKATRICE_ICON_PREVIEW_DIR");
    if (outDir.isEmpty()) {
        GTEST_SKIP() << "set COCKATRICE_ICON_PREVIEW_DIR to write the preview PNGs";
    }
    ensureGuiApplication();
    ASSERT_TRUE(QDir().mkpath(outDir));

    constexpr int MARGIN = 24;
    constexpr int COLUMNS = 2;
    constexpr int CELL_WIDTH = 146;
    constexpr int CELL_HEIGHT = 62;
    constexpr int SECTION_HEIGHT = 22;
    constexpr int PANEL_PADDING = 10;
    constexpr int PANEL_TITLE = 24;
    constexpr int GLYPH = 22;
    constexpr qreal DPR = 2.0;

    // The pages a selected card shows, with every conditional action present
    // so the whole map is visible at once.
    const QList<PreviewPage> pages = {
        PreviewPage{.title = QStringLiteral("Main — on the battlefield"),
                    .sections =
                        {
                            PreviewSection{.title = QStringLiteral("Battlefield"),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("Tap / Untap"),
                                                                 .glyph = "sync",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Skip untapping"),
                                                                 .glyph = "lock",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Clone"),
                                                                 .glyph = "increment",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Attach to card..."),
                                                                 .glyph = "pin",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Draw arrow..."),
                                                                 .glyph = "share",
                                                                 .role = AppColor::MarkedStrong},
                                                   PreviewAction{.label = QStringLiteral("Set annotation..."),
                                                                 .glyph = "pencil",
                                                                 .role = AppColor::MarkedStrong},
                                                   PreviewAction{.label = QStringLiteral("Turn Over"),
                                                                 .glyph = "swap",
                                                                 .role = AppColor::HiddenStrong},
                                                   PreviewAction{.label = QStringLiteral("Peek at card face"),
                                                                 .glyph = "view",
                                                                 .role = AppColor::HiddenStrong},
                                                   PreviewAction{.label = QStringLiteral("Unattach"),
                                                                 .glyph = "arrow_undo",
                                                                 .role = AppColor::NegativeStrong},
                                                   PreviewAction{.label = QStringLiteral("Reduce life"),
                                                                 .glyph = "arrow_down",
                                                                 .role = AppColor::NegativeStrong},
                                               }},
                            PreviewSection{.title = QStringLiteral("Select"),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("Select All"),
                                                                 .glyph = "search",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Select Row"),
                                                                 .glyph = "filter",
                                                                 .role = AppColor::InformationalStrong},
                                               }},
                        }},
        PreviewPage{.title = QStringLiteral("Main — in a zone"),
                    .sections =
                        {
                            PreviewSection{.title = QStringLiteral("Zone"),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("Play"),
                                                                 .glyph = "arrow_right",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Play Face Down"),
                                                                 .glyph = "arrow_right",
                                                                 .role = AppColor::HiddenStrong},
                                                   PreviewAction{.label = QStringLiteral("Clone"),
                                                                 .glyph = "increment",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Attach to card..."),
                                                                 .glyph = "pin",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Draw arrow..."),
                                                                 .glyph = "share",
                                                                 .role = AppColor::MarkedStrong},
                                               }},
                            PreviewSection{.title = QStringLiteral("Select"),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("Select All"),
                                                                 .glyph = "search",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Select Column"),
                                                                 .glyph = "filter",
                                                                 .role = AppColor::InformationalStrong},
                                               }},
                        }},
        PreviewPage{.title = QStringLiteral("Move — card in a zone"),
                    .sections =
                        {
                            PreviewSection{.title = QStringLiteral("Move to"),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("Top of library"),
                                                                 .glyph = "arrow_to_top",
                                                                 .role = AppColor::MovementStrong},
                                                   PreviewAction{.label = QStringLiteral("X from top..."),
                                                                 .glyph = "search",
                                                                 .role = AppColor::MovementStrong},
                                                   PreviewAction{.label = QStringLiteral("Bottom of library"),
                                                                 .glyph = "arrow_to_bottom",
                                                                 .role = AppColor::MovementStrong},
                                                   PreviewAction{.label = QStringLiteral("Table"),
                                                                 .glyph = "player",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Hand"),
                                                                 .glyph = "arrow_left",
                                                                 .role = AppColor::PositiveStrong},
                                                   PreviewAction{.label = QStringLiteral("Graveyard"),
                                                                 .glyph = "arrow_down",
                                                                 .role = AppColor::NegativeStrong},
                                                   PreviewAction{.label = QStringLiteral("Exile"),
                                                                 .glyph = "arrow_right",
                                                                 .role = AppColor::InformationalStrong},
                                               }},
                            PreviewSection{.title = QStringLiteral("Reveal to..."),
                                           .actions =
                                               {
                                                   PreviewAction{.label = QStringLiteral("All players"),
                                                                 .glyph = "share",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Alice"),
                                                                 .glyph = "player",
                                                                 .role = AppColor::InformationalStrong},
                                                   PreviewAction{.label = QStringLiteral("Bob"),
                                                                 .glyph = "player",
                                                                 .role = AppColor::InformationalStrong},
                                               }},
                        }},
    };

    const auto rowsIn = [](const PreviewSection &section) { return (section.actions.size() + COLUMNS - 1) / COLUMNS; };
    const auto panelHeightFor = [&](const PreviewPage &page) {
        int height = PANEL_TITLE + PANEL_PADDING;
        for (const PreviewSection &section : page.sections) {
            height += SECTION_HEIGHT + rowsIn(section) * CELL_HEIGHT;
        }
        return height + PANEL_PADDING;
    };

    const int panelWidth = COLUMNS * CELL_WIDTH + 2 * PANEL_PADDING;
    const int pageCount = static_cast<int>(pages.size());
    int tallestPanel = 0;
    for (const PreviewPage &page : pages) {
        tallestPanel = qMax(tallestPanel, panelHeightFor(page));
    }
    const int logicalWidth = 2 * MARGIN + pageCount * panelWidth + (pageCount - 1) * MARGIN;
    const int logicalHeight = MARGIN + 30 + tallestPanel + MARGIN;

    const QString theme = QStringLiteral("Fabric");
    for (const bool dark : {false, true}) {
        const QString file =
            QDir(QStringLiteral(COCKATRICE_SOURCE_DIR))
                .absoluteFilePath(QStringLiteral("cockatrice/themes/%1/palette-default-%2.toml")
                                      .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));
        const PaletteConfig config = PaletteConfig::fromFile(file);
        ASSERT_TRUE(config.hasPalette()) << theme.toStdString();

        const QPalette palette = config.apply(QPalette());
        const QColor window = palette.color(QPalette::Active, QPalette::Window);
        const QColor text = palette.color(QPalette::Active, QPalette::WindowText);
        // The panel's cells sit on the popup surface; painting them on the
        // button color is what a QToolButton shows when it is not hovered.
        const QColor cell = palette.color(QPalette::Active, QPalette::Button);

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
                         QStringLiteral("%1 — %2 scheme, hover panel action grids")
                             .arg(theme, dark ? QStringLiteral("dark") : QStringLiteral("light")));

        const auto toneFor = [&](AppColor::Role role) {
            return config.appColors.value(role, defaultAppColor(role, dark));
        };

        for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
            const PreviewPage &page = pages[pageIndex];
            const qreal panelLeft = MARGIN + static_cast<qreal>(pageIndex) * (panelWidth + MARGIN);
            const qreal panelTop = MARGIN + 30;
            const int panelHeight = panelHeightFor(page);

            painter.setPen(Qt::NoPen);
            painter.setBrush(cell.darker(dark ? 115 : 103));
            painter.drawRoundedRect(QRectF(panelLeft, panelTop, panelWidth, panelHeight), 8, 8);

            painter.setPen(text);
            painter.setFont(QFont(QStringLiteral("Sans"), 9, QFont::DemiBold));
            painter.drawText(QPointF(panelLeft + PANEL_PADDING, panelTop + 17), page.title);

            qreal y = panelTop + PANEL_TITLE;
            for (const PreviewSection &section : page.sections) {
                painter.setPen(text);
                painter.setFont(QFont(QStringLiteral("Sans"), 8, QFont::DemiBold));
                painter.drawText(QPointF(panelLeft + PANEL_PADDING, y + 15), section.title);
                y += SECTION_HEIGHT;

                const int rows = rowsIn(section);
                for (int row = 0; row < rows; ++row) {
                    for (int column = 0; column < COLUMNS; ++column) {
                        const int index = row * COLUMNS + column;
                        if (index >= section.actions.size()) {
                            break;
                        }
                        const PreviewAction &action = section.actions.at(index);
                        const QRectF cellRect(panelLeft + PANEL_PADDING + column * CELL_WIDTH, y, CELL_WIDTH - 4,
                                              CELL_HEIGHT - 4);

                        painter.setPen(Qt::NoPen);
                        painter.setBrush(cell);
                        painter.drawRoundedRect(cellRect, 5, 5);

                        const QByteArray svg = readIconSvg(action.glyph);
                        ASSERT_FALSE(svg.isEmpty()) << action.glyph;
                        const QPixmap pixmap = renderAtDpr(QSize(GLYPH, GLYPH), DPR, [&](const QSize &pixelSize) {
                            return renderInkGlyph(svg, toneFor(action.role), pixelSize);
                        });
                        painter.drawPixmap(QPointF(cellRect.center().x() - GLYPH / 2.0, cellRect.top() + 7), pixmap);

                        painter.setPen(text);
                        painter.setFont(QFont(QStringLiteral("Sans"), 7));
                        const QFontMetrics metrics(painter.font());
                        const QString label = metrics.elidedText(action.label, Qt::ElideRight, CELL_WIDTH - 12);
                        painter.drawText(QRectF(cellRect.left(), cellRect.bottom() - 20, cellRect.width(), 16),
                                         Qt::AlignHCenter | Qt::AlignVCenter, label);
                    }
                    y += CELL_HEIGHT;
                }
            }
        }

        painter.end();
        const QString path = QDir(outDir).absoluteFilePath(
            QStringLiteral("hover-panel-map-%1.png").arg(dark ? QStringLiteral("dark") : QStringLiteral("light")));
        ASSERT_TRUE(image.save(path)) << path.toStdString();
    }
}
