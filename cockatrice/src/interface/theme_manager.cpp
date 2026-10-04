#include "theme_manager.h"

#include "../../client/settings/cache_settings.h"
#include "pixel_map_generator.h"
#include "theme_config.h"

#include <QApplication>
#include <QChar>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFlags>
#include <QGuiApplication>
#include <QIODevice>
#include <QLatin1Char>
#include <QList>
#include <QMap>
#include <QMessageLogger>
#include <QMetaEnum>
#include <QPalette>
#include <QPixmap>
#include <QPixmapCache>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTextStream>
#include <QWidget>
#include <Qt>
#include <QtVersionChecks>
#include <libcockatrice/settings/paths_settings.h>
#include <qassert.h>
#include <qlogging.h>
#include <qminmax.h>
#include <qnumeric.h>

#define SYSTEM_THEME_NAME "System"
#define LEGACY_SYSTEM_THEME_NAME "Default"
#define FUSION_THEME_NAME "Fusion"
#define STYLE_CSS_NAME "style.css"
#define HANDZONE_BG_NAME "handzone"
#define PLAYERZONE_BG_NAME "playerzone"
#define STACKZONE_BG_NAME "stackzone"
#define TABLEZONE_BG_NAME "tablezone"
static const QColor HANDZONE_BG_DEFAULT = QColor(80, 100, 50);
static const QColor TABLEZONE_BG_DEFAULT = QColor(70, 50, 100);
static const QColor PLAYERZONE_BG_DEFAULT = QColor(200, 200, 200);
static const QColor STACKZONE_BG_DEFAULT = QColor(113, 43, 43);
static const QStringList DEFAULT_RESOURCE_PATHS = {":/resources"};

struct PaletteColorInfo
{
    QPalette::ColorGroup group;
    QPalette::ColorRole role;
    QColor color;
};

[[maybe_unused]] static inline QList<PaletteColorInfo> queryAllPaletteColors(const QPalette &palette = qApp->palette())
{
    QList<PaletteColorInfo> colors;

    // Iterate through relevant color groups (Active, Disabled, Inactive)
    const QList<QPalette::ColorGroup> groups = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

    for (auto group : groups) {
        // Iterate through all color roles (excluding NoRole and NColorRoles)
        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            auto role = static_cast<QPalette::ColorRole>(r);
            if (role == QPalette::NoRole) {
                continue;
            }

            PaletteColorInfo info;
            info.group = group;
            info.role = role;
            info.color = palette.color(group, role);
            colors.append(info);
        }
    }

    return colors;
}

// Pretty print version
[[maybe_unused]] static inline void printPaletteColors(const QPalette &palette = qApp->palette())
{
    QMetaEnum groupEnum = QMetaEnum::fromType<QPalette::ColorGroup>();
    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    const QList<QPalette::ColorGroup> groups = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

    for (auto group : groups) {
        qInfo() << "\n===========" << groupEnum.valueToKey(group) << "===========";

        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            auto role = static_cast<QPalette::ColorRole>(r);
            if (role == QPalette::NoRole) {
                continue;
            }

            QColor color = palette.color(group, role);
            qInfo().nospace() << qPrintable(QString("%1").arg(roleEnum.valueToKey(role), -20)) << " : "
                              << qPrintable(color.name(QColor::HexArgb)) << " (RGBA: " << color.red() << ", "
                              << color.green() << ", " << color.blue() << ", " << color.alpha() << ")";
        }
    }
}

static QString usableDefaultStyle(const QString &style)
{
    // The Windows 11 native style is broken: dragging cards across zones can
    // shrink the board to a tiny grey window that is unfixable without
    // rejoining. It is never usable, so guard against it no matter how it was
    // requested (OS default or an explicit "windows11" theme choice) and fall
    // back to the Vista style.
    return style.compare("windows11", Qt::CaseInsensitive) == 0 ? QStringLiteral("windowsvista") : style;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    defaultStyleName = usableDefaultStyle(qApp->style()->objectName());
    // Capture the untouched application palette before any theme is applied.
    defaultPalette = qApp->palette();
    ensureThemeDirectoryExists();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        // Reload so scheme-qualified assets and palettes follow the OS, but do
        // NOT recapture defaultPalette: qApp->palette() already carries the
        // currently-applied theme palette at this point, so recapturing it
        // would contaminate the base for every later theme switch.
        themeChangedSlot();
    });
#endif
    connect(&SettingsCache::instance(), &SettingsCache::themeChanged, this, &ThemeManager::themeChangedSlot);
    themeChangedSlot();
}

// Copy every file of sourceDir into targetDir without ever overwriting an
// existing file, recursing into subdirectories (zones/, backgrounds/, ...).
// Returns false as soon as an entry could not be copied, leaving the source
// untouched so the caller can retry later.
static bool mergeDirWithoutOverwrite(const QString &sourceDir, const QString &targetDir)
{
    const QDir target(targetDir);
    bool merged = true;

    const QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden;
    for (const QFileInfo &entry : QDir(sourceDir).entryInfoList(filters)) {
        const QString targetEntry = target.absoluteFilePath(entry.fileName());
        if (entry.isDir()) {
            if (!QDir().mkpath(targetEntry) || !mergeDirWithoutOverwrite(entry.absoluteFilePath(), targetEntry)) {
                merged = false;
            }
        } else if (!target.exists(entry.fileName()) && !QFile::copy(entry.absoluteFilePath(), targetEntry)) {
            qCWarning(ThemeManagerLog) << "Could not migrate theme file:" << entry.absoluteFilePath();
            merged = false;
        }
    }

    return merged;
}

void ThemeManager::migrateLegacyThemeDir(const QString &themesPath, const QString &currentThemeName)
{
    const QString legacyPath = QDir(themesPath).absoluteFilePath(LEGACY_SYSTEM_THEME_NAME);
    if (!QDir(legacyPath).exists()) {
        return;
    }

    // Only the renamed theme's own directory is migrated. A user sitting on a
    // different theme may well have hand-made a "Default" folder of their own,
    // and silently folding it into "System" would be rude. The check accepts
    // both the pre-rename name and the post-rename one, since a user who
    // already ran the rename keeps their settings on "System" while their
    // customised directory is still called "Default".
    if (currentThemeName != LEGACY_SYSTEM_THEME_NAME && currentThemeName != SYSTEM_THEME_NAME) {
        return;
    }

    const QString targetPath = QDir(themesPath).absoluteFilePath(SYSTEM_THEME_NAME);
    if (!QDir(targetPath).exists() && QDir().rename(legacyPath, targetPath)) {
        qCInfo(ThemeManagerLog) << "Migrated customised theme directory" << LEGACY_SYSTEM_THEME_NAME << "to"
                                << SYSTEM_THEME_NAME;
        return;
    }

    // The rename is only possible when the target does not exist yet. When it
    // does (a partial earlier upgrade, or a user who re-customised the theme
    // after the rename), fall back to merging so nothing is lost: existing
    // "System" files win, legacy files only fill gaps. Because every legacy
    // file either already exists at the target or is copied there, removing
    // the legacy directory afterwards cannot drop user data.
    if (!mergeDirWithoutOverwrite(legacyPath, targetPath)) {
        qCWarning(ThemeManagerLog) << "Could not fully migrate theme directory" << legacyPath << "; keeping it for a"
                                   << "later retry";
        return;
    }
    if (!QDir(legacyPath).removeRecursively()) {
        qCWarning(ThemeManagerLog) << "Migrated theme directory but could not remove the legacy one:" << legacyPath;
        return;
    }
    qCInfo(ThemeManagerLog) << "Merged customised theme directory" << LEGACY_SYSTEM_THEME_NAME << "into"
                            << SYSTEM_THEME_NAME;
}

void ThemeManager::ensureThemeDirectoryExists()
{
    auto &settings = SettingsCache::instance();

    // Carry the user's theme customisations (zone graphics, stylesheets,
    // palettes, theme.cfg) across before the stored name is rewritten.
    migrateLegacyThemeDir(settings.paths().getThemesPath(), settings.getThemeName());

    // Migrate the old "Default" theme name to "System"
    if (settings.getThemeName() == LEGACY_SYSTEM_THEME_NAME) {
        settings.setThemeName(SYSTEM_THEME_NAME);
    }

    if (settings.getThemeName().isEmpty() || !getAvailableThemes().contains(settings.getThemeName())) {
        qCInfo(ThemeManagerLog) << "Theme name not set, setting default value";
        settings.setThemeName(FUSION_THEME_NAME);
    }
}

bool ThemeManager::isDarkMode(const QString &themeDirPath) const
{
    ThemeConfig themeConfig = ThemeConfig::fromThemeDir(themeDirPath);
    if (themeConfig.colorScheme.compare("Dark", Qt::CaseInsensitive) == 0) {
        return true;
    } else if (themeConfig.colorScheme.compare("Light", Qt::CaseInsensitive) == 0) {
        return false;
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        bool osDark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
        bool osDark = false;
#endif
        return osDark;
    }
}

QString ThemeManager::schemeVariantPath(QStringView prefix) const
{
    static const QStringList formats = {QStringLiteral(".png"), QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
                                        QStringLiteral(".svg")};
    const QString scheme = isDarkMode(currentThemePath) ? QStringLiteral("dark") : QStringLiteral("light");
    const QString variantStem = prefix.toString() + QLatin1Char('-') + scheme;

    for (const QString &format : formats) {
        if (QFileInfo::exists(QStringLiteral("theme:") + variantStem + format)) {
            return variantStem + format;
        }
    }
    return QString();
}

QString ThemeManager::assetPath(QStringView prefix) const
{
    // Probe order mirrors tryLoadImage: a theme may override the default SVG
    // with a raster of the same stem, so raster wins over SVG within a stem.
    static const QStringList formats = {QStringLiteral(".png"), QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
                                        QStringLiteral(".svg")};

    auto findExisting = [](const QString &stem) {
        for (const QString &format : formats) {
            if (QFileInfo::exists(QStringLiteral("theme:") + stem + format)) {
                return stem + format;
            }
        }
        return QString();
    };

    // Prefer the scheme-qualified variant when it exists, else the plain
    // asset as the super fallback. Both return the resolved path including
    // its file extension so callers can load it directly.
    const QString variant = schemeVariantPath(prefix);
    if (!variant.isEmpty()) {
        return variant;
    }
    const QString resolvedPlain = findExisting(prefix.toString());
    return resolvedPlain.isEmpty() ? prefix.toString() : resolvedPlain;
}

// Probe whether a directory is truly writable by trying to create and remove a
// temporary file. QFileInfo::isWritable() on a directory is unreliable (notably
// on Windows where UAC VirtualStore can make a system dir appear writable).
bool ThemeManager::isDirReallyWritable(const QString &dirPath)
{
    const QString probe = QDir(dirPath).absoluteFilePath(".cockatrice_write_test");
    QFile f(probe);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    f.close();
    f.remove();
    return true;
}

QString ThemeManager::writableThemeDir(const QString &themeName)
{
    // All theme writes go to the user themes directory regardless of whether
    // the resolved (system) theme directory happens to be writable. Even when a
    // write would succeed in-place, routing it to the user directory keeps the
    // install intact and guarantees changes survive upgrades.
    const QString dirPath = QDir(SettingsCache::instance().paths().getThemesPath()).absoluteFilePath(themeName);
    if (!QDir().mkpath(dirPath)) {
        qWarning() << "Failed to create theme save directory:" << dirPath;
    }
    return dirPath;
}

// System (read-only) themes location, relative to the application binary.
static QString systemThemesBasePath()
{
    QString base = qApp->applicationDirPath();
#ifdef Q_OS_MAC
    base += "/../Resources/themes";
#elif defined(Q_OS_WIN)
    base += "/themes";
#else // linux
    base += "/../share/cockatrice/themes";
#endif
    return base;
}

QStringMap &ThemeManager::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // load themes from user profile dir
    dir.setPath(SettingsCache::instance().paths().getThemesPath());

    availableThemes.insert(SYSTEM_THEME_NAME, dir.absoluteFilePath("System"));
    availableThemes.insert(FUSION_THEME_NAME, dir.absoluteFilePath("Fusion"));

    for (QString themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName)) {
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
        }
    }

    // Load themes from Cockatrice system dir
    dir.setPath(systemThemesBasePath());

    for (QString themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName)) {
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
        }
    }

    return availableThemes;
}

QBrush ThemeManager::loadBrush(QString fileName, QColor fallbackColor)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:" + assetPath(QStringLiteral("zones/") + fileName));
    if (tmp.isNull()) {
        brush.setColor(fallbackColor);
        brush.setStyle(Qt::SolidPattern);
    } else {
        brush.setTexture(tmp);
    }

    return brush;
}

QBrush ThemeManager::loadExtraBrush(QString fileName, QBrush &fallbackBrush)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:" + assetPath(QStringLiteral("zones/") + fileName));

    if (tmp.isNull()) {
        brush = fallbackBrush;
    } else {
        brush.setTexture(tmp);
    }

    return brush;
}

ThemeConfig ThemeManager::loadGlobalConfig(const QString &themeDirPath)
{
    return ThemeConfig::fromThemeDir(themeDirPath);
}

bool ThemeManager::saveGlobalConfig(const QString &themeDirPath, const ThemeConfig &cfg)
{
    return cfg.save(themeDirPath);
}

PaletteConfig ThemeManager::loadPaletteConfig(const QString &themeDirPath, const QString &colorScheme)
{
    if (themeDirPath.isEmpty()) {
        return {};
    }
    return PaletteConfig::fromScheme(themeDirPath, colorScheme);
}

bool ThemeManager::savePaletteConfig(const QString &themeDirPath, const QString &colorScheme, const PaletteConfig &cfg)
{
    if (themeDirPath.isEmpty()) {
        return false;
    }

    QDir dir(themeDirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile f(dir.absoluteFilePath(PaletteConfig::fileName(colorScheme)));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream(&f) << cfg.toToml();
    return true;
}

PaletteConfig ThemeManager::loadDefaultPaletteConfig(const QString &themeDirPath,
                                                     const QString &themeName,
                                                     const QString &colorScheme)
{
    PaletteConfig cfg = PaletteConfig::fromDefault(themeDirPath, colorScheme);
    if (!cfg.hasPalette()) {
        // The shipped default may live in the system theme directory rather
        // than the resolved (user) theme directory, so built-in themes still
        // get their curated defaults.
        cfg = PaletteConfig::fromDefault(QDir(systemThemesBasePath()).absoluteFilePath(themeName), colorScheme);
    }
    return cfg;
}

bool ThemeManager::commitPalette(const QString &themeDirPath, const QString &colorScheme, const PaletteConfig &cfg)
{
    if (!savePaletteConfig(themeDirPath, colorScheme, cfg)) {
        return false;
    }

    ThemeConfig globalCfg = ThemeConfig::fromThemeDir(themeDirPath);
    globalCfg.colorScheme = colorScheme;
    globalCfg.save(themeDirPath);

    return true;
}

void ThemeManager::setColorScheme(const QString &scheme)
{
    const QString dirPath = writableThemeDir(SettingsCache::instance().getThemeName());
    ThemeConfig cfg = ThemeConfig::fromThemeDir(dirPath);

    cfg.colorScheme = scheme;

    cfg.save(dirPath);
    reloadCurrentTheme();
}

void ThemeManager::setStyleName(const QString &styleName)
{
    const QString dirPath = writableThemeDir(SettingsCache::instance().getThemeName());
    ThemeConfig cfg = ThemeConfig::fromThemeDir(dirPath);

    cfg.styleName = styleName;

    cfg.save(dirPath);
    reloadCurrentTheme();
}

void ThemeManager::reloadCurrentTheme()
{
    themeChangedSlot();
}

void ThemeManager::previewPalette(const PaletteConfig &cfg, const QString &scheme)
{
    const QString themeName = SettingsCache::instance().getThemeName();
    const QString dirPath = getAvailableThemes().value(themeName);
    const ThemeConfig themeCfg = ThemeConfig::fromThemeDir(dirPath);
    applyStyleAndPalette(themeName, themeCfg, cfg, scheme);
}

void ThemeManager::applyStyleAndPalette(const QString &themeName,
                                        const ThemeConfig &themeCfg,
                                        const PaletteConfig &palCfg,
                                        const QString &activeScheme)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    Q_UNUSED(activeScheme)
#endif
    QString styleName = themeCfg.styleName;
    if (styleName.isEmpty() || styleName.compare("System", Qt::CaseInsensitive) == 0) {
        if (themeName == FUSION_THEME_NAME) {
            styleName = "Fusion";
        } else {
            styleName = usableDefaultStyle(defaultStyleName);
        }
    }

    // The Windows 11 style is broken even when selected explicitly in a theme,
    // so sanitize the resolved name here rather than trusting the theme config.
    styleName = usableDefaultStyle(styleName);

    QStyle *style = QStyleFactory::create(styleName);
    if (!style) {
        style = QStyleFactory::create(usableDefaultStyle(defaultStyleName));
    }

    // Base palette
    QPalette base;
    if (styleName.compare("Fusion", Qt::CaseInsensitive) == 0) {
        base = style->standardPalette();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
        if (activeScheme == "Dark") {
            base.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        }
#endif
    } else {
        // Use the pristine startup palette rather than qApp->palette(): the
        // latter may already carry a previously-applied custom (e.g. dark)
        // palette, which would otherwise persist when switching to a scheme
        // that supplies no palette of its own.
        base = defaultPalette;
    }

    // Overlay custom palette colours
    if (palCfg.hasPalette()) {
        base = palCfg.apply(base);
    }

    // Palette BEFORE style — setStyle() triggers a synchronous repolish of all
    // widgets immediately. If the palette isn't set yet at that point, every
    // widget gets polished against the stale colours, requiring a second apply
    // to fully resolve. Setting palette first means setStyle's repolish cascade
    // already sees the correct colours.
    qApp->setPalette(base);
    qApp->setStyle(style);

    currentAppColors = palCfg.appColors;

    // Force every widget to re-polish and repaint immediately rather than
    // waiting for natural expose events, which produces a patchwork of old
    // and new colours during a live preview.
    // Note: we do NOT call widget->setPalette(base) here — qApp->setPalette()
    // already propagates to all widgets that haven't explicitly overridden their
    // palette (WA_SetPalette not set). Calling it unconditionally would clobber
    // intentional per-widget palette customisations across the whole app.
    for (QWidget *widget : qApp->allWidgets()) {
        style->unpolish(widget);
        style->polish(widget);
        widget->update();
    }

    emit paletteChanged();
}

QColor ThemeManager::appColor(AppColor::Role role) const
{
    const auto it = currentAppColors.constFind(role);
    if (it != currentAppColors.constEnd()) {
        return it.value();
    }

    // QPalette::Accent was introduced in Qt 6.6 and several shipped palettes
    // set it to a value barely distinguishable from Window, so it is not a
    // reliable accent source. The selection highlight is the stable accent
    // (Accent defaults to Highlight when unset), and deriving from it
    // unconditionally keeps every Qt version rendering identically.
    const QColor accent = qApp->palette().color(QPalette::Active, QPalette::Highlight);

    if (role == AppColor::AccentSoft) {
        constexpr int SOFT_SATURATION_PERCENT = 70;
        constexpr int SOFT_LIGHTNESS_OFFSET = 60;

        // Light end of the gradient: same hue, softened and lightened
        return QColor::fromHsl(qMax(0, accent.hslHue()),
                               qBound(0, qRound(accent.hslSaturation() * SOFT_SATURATION_PERCENT / 100.0), 255),
                               qBound(0, accent.lightness() + SOFT_LIGHTNESS_OFFSET, 255));
    }

    return accent;
}

void ThemeManager::themeChangedSlot()
{
    QString themeName = SettingsCache::instance().getThemeName();
    QString dirPath = getAvailableThemes().value(themeName);
    currentThemePath = dirPath;
    QDir dir(dirPath);

    // CSS — prefer the scheme-qualified stylesheet (style-dark.css /
    // style-light.css) when present, else the plain style.css as fallback.
    if (!dirPath.isEmpty()) {
        const QString scheme = isDarkMode(dirPath) ? QStringLiteral("dark") : QStringLiteral("light");
        const QString schemeCss = QFileInfo(QStringLiteral(STYLE_CSS_NAME)).completeBaseName() + QLatin1Char('-') +
                                  scheme + QStringLiteral(".css");
        if (dir.exists(schemeCss)) {
            qApp->setStyleSheet("file:///" + dir.absoluteFilePath(schemeCss));
        } else if (dir.exists(STYLE_CSS_NAME)) {
            qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
        } else {
            qApp->setStyleSheet("");
        }
    } else {
        qApp->setStyleSheet("");
    }

    // load theme.cfg for style + scheme preference
    ThemeConfig themeCfg = ThemeConfig::fromThemeDir(dirPath);

    // Resolve active scheme:
    // theme.cfg says Dark/Light → use that
    // theme.cfg says System or is absent → follow the OS
    QString activeScheme = isDarkMode(dirPath) ? "Dark" : "Light";

    // ── Load palette: custom first, then theme default ────────────────────
    PaletteConfig palette = PaletteConfig::fromScheme(dirPath, activeScheme);
    const PaletteConfig themeDefault = ThemeManager::loadDefaultPaletteConfig(dirPath, themeName, activeScheme);
    if (palette.hasPalette()) {
        // A custom palette written before [AppColors] existed carries no app
        // colors; merge the theme's shipped defaults so the identity colors
        // survive (hasPalette() counts an app-colors-only file as a palette,
        // so those are kept wholesale and never reach here empty).
        for (auto it = themeDefault.appColors.cbegin(); it != themeDefault.appColors.cend(); ++it) {
            if (!palette.appColors.contains(it.key())) {
                palette.appColors.insert(it.key(), it.value());
            }
        }
    } else {
        palette = themeDefault;
    }

    applyStyleAndPalette(themeName, themeCfg, palette, activeScheme);

    QStringList resources;
    if (!dirPath.isEmpty()) {
        resources << dir.absolutePath();
    }

    // When the resolved dir is a user copy (e.g. user/<theme>), also
    // include the system theme dir as a fallback so shipped assets like
    // zones/*.png and style.css still resolve for themes that ship only
    // those files (e.g. Leather, Plasma, Fabric, VelvetMarble).
    const QString sysPath = QDir(systemThemesBasePath()).absoluteFilePath(themeName);
    if (sysPath != dirPath && QDir(sysPath).exists()) {
        resources << sysPath;
    }

    resources << DEFAULT_RESOURCE_PATHS;

    QDir::setSearchPaths("theme", resources);

    brushes[Role::Hand] = loadBrush(HANDZONE_BG_NAME, HANDZONE_BG_DEFAULT);

    brushes[Role::Table] = loadBrush(TABLEZONE_BG_NAME, TABLEZONE_BG_DEFAULT);

    brushes[Role::Player] = loadBrush(PLAYERZONE_BG_NAME, PLAYERZONE_BG_DEFAULT);

    brushes[Role::Stack] = loadBrush(STACKZONE_BG_NAME, STACKZONE_BG_DEFAULT);
    for (auto &brushCache : brushesCache) {
        brushCache.clear();
    }

    QPixmapCache::clear();
    clearPixmapGeneratorCaches();

    emit themeChanged();
}

static QString roleBgName(ThemeManager::Role role)
{
    switch (role) {
        case ThemeManager::Hand:
            return HANDZONE_BG_NAME;

        case ThemeManager::Player:
            return PLAYERZONE_BG_NAME;

        case ThemeManager::Stack:
            return STACKZONE_BG_NAME;

        case ThemeManager::Table:
            return TABLEZONE_BG_NAME;

        default:
            Q_ASSERT(false);
            return {};
    }
}

QBrush &ThemeManager::getBgBrush(Role role)
{
    return brushes[role];
}

QBrush ThemeManager::getExtraBgBrush(Role role, int zoneId)
{
    if (zoneId <= 0) {
        return getBgBrush(role);
    }

    QBrushMap &brushCache = brushesCache[role];

    if (!brushCache.contains(zoneId)) {
        QBrush brush = loadExtraBrush(roleBgName(role) + QString::number(zoneId), getBgBrush(role));
        brushCache.insert(zoneId, brush);
        return brush;
    }

    return brushCache.value(zoneId);
}
