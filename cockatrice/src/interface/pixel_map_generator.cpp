#include "pixel_map_generator.h"

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QImageReader>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

#define DEFAULT_COLOR_UNREGISTERED "#32c8ec";
#define DEFAULT_COLOR_REGISTERED "#5ed900";
#define DEFAULT_COLOR_MODERATOR_LEFT "#ffffff";
#define DEFAULT_COLOR_MODERATOR_RIGHT "#000000";
#define DEFAULT_COLOR_ADMIN "#ff2701";

/**
 * Clamps an svg render size so that rendering does not exceed a multiple of the requested size.
 *
 * Rendering at the full native size of an svg just to scale it down afterwards wastes memory,
 * and canvases with extreme coordinates can exceed Qt's rasterizer coordinate limit which makes
 * Qt silently drop shapes from the rendered image.
 *
 * @param renderSize The size the svg would be rendered at.
 * @param requestedSize The size that was actually requested.
 *
 * @return A size with the aspect ratio of renderSize whose longest side is at most four times
 * the longest side of requestedSize.
 */
static QSize capRenderSize(const QSize &renderSize, const QSize &requestedSize)
{
    const int longestRequestedSide = qMax(requestedSize.width(), requestedSize.height());
    if (longestRequestedSide <= 0) {
        return renderSize;
    }

    const int longestRenderSide = qMax(renderSize.width(), renderSize.height());
    const qreal scale = qMin<qreal>(1.0, static_cast<qreal>(longestRequestedSide * 4) / longestRenderSide);
    return QSize(qMax(1, static_cast<int>(renderSize.width() * scale)),
                 qMax(1, static_cast<int>(renderSize.height() * scale)));
}

/**
 * Loads in an svg from file and scales it without affecting image quality.
 *
 * @param svgPath The path to the svg file, with file extension.
 * @param size The desired size of the pixmap.
 * @param expandOnly If true, then keep the size of the initial pixmap to at least the svg size.
 *
 * @return The svg loaded into a Pixmap with the given size, or an empty Pixmap if the loading failed.
 */
static QPixmap loadSvg(const QString &svgPath, const QSize &size, bool expandOnly = false)
{
    QSvgRenderer svgRenderer(svgPath);

    if (!svgRenderer.isValid()) {
        qCWarning(PixelMapGeneratorLog) << "Failed to load" << svgPath;
        return {};
    }

    // If expandOnly, make sure the pixmap is at least as large as the svg, so that we don't lose any detail.
    // QIcon.pixmap(size) will automatically scale down the image, but it won't scale it up.
    QSize pixmapSize = expandOnly ? svgRenderer.defaultSize().expandedTo(size) : size;
    if (expandOnly) {
        pixmapSize = capRenderSize(pixmapSize, size);
    }
    QPixmap pix(pixmapSize);
    pix.fill(Qt::transparent);

    QPainter pixPainter(&pix);
    svgRenderer.render(&pixPainter);

    // Converting the pixmap to a QIcon and back is the easiest way to scale down a svg without affecting image quality
    if (expandOnly) {
        return QIcon(pix).pixmap(size);
    }

    return pix;
}

/**
 * Try to load path image from non-SVG formats, otherwise fall back to SVG.
 * This is to allow custom themes to support non-SVG format type overrides, since SVG requires custom loading.
 * @param path The path to the file, with no file extension. File formats will be automatically detected.
 * @param size The desired size of the pixmap.
 * @param expandOnly If true, then keep the size of the initial pixmap to at least the size (Only relevant if SVG).
 *
 * @return The loaded image into a Pixmap with the given size, or an empty Pixmap if the loading failed.
 */
static QPixmap tryLoadImage(const QString &path, const QSize &size, bool expandOnly = false)
{
    const auto formats = {"png", "jpg"};

    QPixmap returnPixmap;
    for (const auto &format : formats) {
        if (returnPixmap.load(path, format)) {
            return returnPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    return loadSvg(path + ".svg", size, expandOnly);
}

QMap<QString, QPixmap> PhasePixmapGenerator::pmCache;

QPixmap PhasePixmapGenerator::generatePixmap(int height, QString name)
{
    QString key = name + QString::number(height);
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    QPixmap pixmap = tryLoadImage("theme:phases/" + name, QSize(height, height));

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> CounterPixmapGenerator::pmCache;

QPixmap CounterPixmapGenerator::generatePixmap(int height, QString name, bool highlight)
{
    // The colorless counter is named "x" by the server but the file is named "general.svg"
    if (name == "x") {
        name = "general";
    }

    if (highlight) {
        name.append("_highlight");
    }
    QString key = name + QString::number(height);
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    QPixmap pixmap = tryLoadImage("theme:counters/" + name, QSize(height, height));

    // fall back to colorless counter if the name can't be found
    if (pixmap.isNull()) {
        name = "general";
        if (highlight) {
            name.append("_highlight");
        }
        pixmap = tryLoadImage("theme:counters/" + name, QSize(height, height));
    }

    pmCache.insert(key, pixmap);
    return pixmap;
}

QPixmap PingPixmapGenerator::generatePixmap(int size, int value, int max)
{
    int key = size * 1000000 + max * 1000 + value;
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QColor color;
    if ((max == -1) || (value == -1)) {
        color = Qt::black;
    } else {
        color.setHsv(120 * (1.0 - ((double)value / max)), 255, 255);
    }

    QRadialGradient g(QPointF((double)pixmap.width() / 2, (double)pixmap.height() / 2),
                      qMin(pixmap.width(), pixmap.height()) / 2.0);
    g.setColorAt(0, color);
    g.setColorAt(1, Qt::transparent);
    painter.fillRect(0, 0, pixmap.width(), pixmap.height(), QBrush(g));

    pmCache.insert(key, pixmap);

    return pixmap;
}

QMap<int, QPixmap> PingPixmapGenerator::pmCache;

QPixmap CountryPixmapGenerator::generatePixmap(int height, const QString &countryCode)
{
    if (countryCode.size() != 2) {
        return QPixmap();
    }
    QString key = countryCode + QString::number(height);
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    int width = height * 2;
    QPixmap pixmap = tryLoadImage("theme:countries/" + countryCode.toLower(), QSize(width, height), true);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);

    // width/height offset was determined through trial-and-error
#ifdef Q_OS_MACOS
    painter.drawRect(0, 0, pixmap.width() / 2, pixmap.height() / 2);
#else
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);
#endif

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> CountryPixmapGenerator::pmCache;

/**
 * Updates tags in the svg
 *
 * @param elem The svg
 * @param tagName tag with attribute to update
 * @param attrName attribute to be updated
 * @param idName id that the tag has to match
 * @param attrValue the value to update the attribute to
 */
static void setAttrRecur(QDomElement &elem,
                         const QString &tagName,
                         const QString &attrName,
                         const QString &idName,
                         const QString &attrValue);

void setAttrRecur(QDomElement &elem,
                  const QString &tagName,
                  const QString &attrName,
                  const QString &idName,
                  const QString &attrValue)
{
    if (elem.tagName().compare(tagName) == 0) {
        if (elem.attribute("id").compare(idName) == 0) {
            elem.setAttribute(attrName, attrValue);
        }
    }

    for (int i = 0; i < elem.childNodes().count(); i++) {
        if (!elem.childNodes().at(i).isElement()) {
            continue;
        }
        auto docElem = elem.childNodes().at(i).toElement();
        setAttrRecur(docElem, tagName, attrName, idName, attrValue);
    }
}

/**
 * Loads the usericon svg and fills in its colors.
 * The image is kept as a QIcon to preserve the image quality.
 *
 * Call icon.pixmap(w, h) in order to convert this icon into a pixmap with the given dimensions.
 * Avoid scaling the pixmap in other ways, as that destroys image quality.
 *
 * @param minSize If the dimensions of the source svg is smaller than this, then it will be scaled up to this size
 */
static QIcon loadAndColorSvg(const QString &iconPath,
                             const QString &colorLeft,
                             const std::optional<QString> &colorRight,
                             const int minSize)
{
    QFile file(iconPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(PixelMapGeneratorLog) << "Unable to open" << iconPath;
        return {};
    }

    const auto &baData = file.readAll();
    QDomDocument doc;
    doc.setContent(baData);

    auto docElem = doc.documentElement();

    setAttrRecur(docElem, "path", "fill", "left", colorLeft);
    if (colorRight.has_value()) {
        setAttrRecur(docElem, "path", "fill", "right", colorRight.value());
    }

    QSvgRenderer svgRenderer(doc.toByteArray());

    const QSize pixmapSize =
        capRenderSize(svgRenderer.defaultSize().expandedTo(QSize(minSize, minSize)), QSize(minSize, minSize));
    QPixmap pix(pixmapSize);
    pix.fill(Qt::transparent);

    QPainter pixPainter(&pix);
    svgRenderer.render(&pixPainter);

    return QIcon(pix);
}

QPixmap UserLevelPixmapGenerator::generatePixmap(int height,
                                                 UserLevelFlags userLevel,
                                                 ServerInfo_User::PawnColorsOverride pawnColorsOverride,
                                                 bool isBuddy,
                                                 const QString &privLevel)
{
    return generateIcon(height, userLevel, pawnColorsOverride, isBuddy, privLevel).pixmap(height, height);
}

QIcon UserLevelPixmapGenerator::generateIcon(int minHeight,
                                             UserLevelFlags userLevel,
                                             ServerInfo_User::PawnColorsOverride pawnColorsOverride,
                                             bool isBuddy,
                                             const QString &privLevel)
{
    std::optional<QString> colorLeft = std::nullopt;
    if (pawnColorsOverride.has_left_side()) {
        colorLeft = QString::fromStdString(pawnColorsOverride.left_side());
    }

    std::optional<QString> colorRight = std::nullopt;
    if (pawnColorsOverride.has_right_side()) {
        colorRight = QString::fromStdString(pawnColorsOverride.right_side());
    }

    QString key = QString::number(minHeight * 10000) + ":" + static_cast<short>(userLevel) + ":" +
                  static_cast<short>(isBuddy) + ":" + privLevel.toLower() + ":" + colorLeft.value_or("") + ":" +
                  colorRight.value_or("");

    if (iconCache.contains(key)) {
        return iconCache.value(key);
    }

    QIcon icon = colorLeft.has_value()
                     ? generateIconWithColorOverride(minHeight, isBuddy, userLevel, privLevel, colorLeft, colorRight)
                     : generateIconDefault(minHeight, userLevel, isBuddy, privLevel);

    iconCache.insert(key, icon);
    return icon;
}

static QString getIconType(const bool isBuddy, const UserLevelFlags &userLevelFlags, const QString &privLevel)
{
    if (isBuddy) {
        return "star";
    }

    if (userLevelFlags.testFlag(ServerInfo_User_UserLevelFlag_IsJudge)) {
        return "pawn_judge";
    }

    if (!privLevel.isEmpty() && privLevel.toLower() != "none") {
        return QString("pawn_%1").arg(privLevel.toLower());
    }

    return "pawn";
}

QIcon UserLevelPixmapGenerator::generateIconDefault(int height,
                                                    UserLevelFlags userLevel,
                                                    bool isBuddy,
                                                    const QString &privLevel)
{
    const auto &iconType = getIconType(isBuddy, userLevel, privLevel);

    QString arity = "single";
    QString colorLeft;
    std::optional<QString> colorRight = std::nullopt;

    if (userLevel.testFlag(ServerInfo_User::IsAdmin)) {
        colorLeft = DEFAULT_COLOR_ADMIN;
    } else if (userLevel.testFlag(ServerInfo_User::IsModerator)) {
        colorLeft = DEFAULT_COLOR_MODERATOR_LEFT;
        colorRight = DEFAULT_COLOR_MODERATOR_RIGHT;
        arity = "double";
    } else if (userLevel.testFlag(ServerInfo_User::IsRegistered)) {
        colorLeft = DEFAULT_COLOR_REGISTERED;
    } else {
        colorLeft = DEFAULT_COLOR_UNREGISTERED;
    }

    const auto &iconPath = QString("theme:usericons/%1_%2.svg").arg(iconType, arity);
    return loadAndColorSvg(iconPath, colorLeft, colorRight, height);
}

QIcon UserLevelPixmapGenerator::generateIconWithColorOverride(int height,
                                                              bool isBuddy,
                                                              const UserLevelFlags &userLevelFlags,
                                                              const QString &privLevel,
                                                              const std::optional<QString> &colorLeft,
                                                              const std::optional<QString> &colorRight)
{
    const auto &iconType = getIconType(isBuddy, userLevelFlags, privLevel);
    const QString &arity = colorRight.has_value() ? "double" : "single";
    const auto &iconPath = QString("theme:usericons/%1_%2.svg").arg(iconType, arity);
    return loadAndColorSvg(iconPath, colorLeft.value(), colorRight, height);
}

QMap<QString, QIcon> UserLevelPixmapGenerator::iconCache;

QPixmap LockPixmapGenerator::generatePixmap(int height)
{

    int key = height;
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    QPixmap pixmap = tryLoadImage("theme:icons/lock", QSize(height, height), true);
    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<int, QPixmap> LockPixmapGenerator::pmCache;

QPixmap DropdownIconPixmapGenerator::generatePixmap(int height, bool expanded)
{
    QString key = QString::number(expanded) + ":" + QString::number(height);
    if (pmCache.contains(key)) {
        return pmCache.value(key);
    }

    QString name = expanded ? "dropdown_expanded" : "dropdown_collapsed";
    QPixmap pixmap = tryLoadImage("theme:icons/" + name, QSize(height, height), true);

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> DropdownIconPixmapGenerator::pmCache;

namespace
{
/// Longest side mana symbols are rendered at before being scaled to their final size.
constexpr int MASTER_ICON_SIZE = 128;

QString manaSymbolCacheKey(const QString &symbol, const QSize &size)
{
    return symbol + QLatin1Char('|') + QString::number(size.width()) + QLatin1Char('x') +
           QString::number(size.height());
}
} // namespace

const QPixmap &ManaSymbolPixmapGenerator::masterIcon(const QString &symbol)
{
    auto it = masterCache.constFind(symbol);
    if (it != masterCache.constEnd()) {
        return it.value();
    }

    QImageReader reader("theme:icons/mana/" + symbol);
    QSize sourceSize = reader.size();
    if (!sourceSize.isEmpty()) {
        sourceSize.scale(QSize(MASTER_ICON_SIZE, MASTER_ICON_SIZE), Qt::KeepAspectRatio);
        reader.setScaledSize(sourceSize);
    }
    const QPixmap rendered = QPixmap::fromImageReader(&reader);

    return masterCache.insert(symbol, rendered).value();
}

QPixmap ManaSymbolPixmapGenerator::generatePixmap(const QString &symbol, const QSize &size)
{
    const QString key = manaSymbolCacheKey(symbol, size);
    auto it = scaledCache.constFind(key);
    if (it != scaledCache.constEnd()) {
        return it.value();
    }

    const QPixmap &icon = masterIcon(symbol);
    if (icon.isNull()) {
        return {};
    }

    QPixmap scaled = icon.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledCache.insert(key, scaled);
    return scaled;
}

QHash<QString, QPixmap> ManaSymbolPixmapGenerator::masterCache;
QHash<QString, QPixmap> ManaSymbolPixmapGenerator::scaledCache;

QPixmap loadColorAdjustedPixmap(const QString &name)
{
    if (qApp->palette().windowText().color().lightness() > 200) {
        QImage img(name);
        img.invertPixels();
        QPixmap result;
        result.convertFromImage(img);
        return result;
    } else {
        return QPixmap(name);
    }
}
