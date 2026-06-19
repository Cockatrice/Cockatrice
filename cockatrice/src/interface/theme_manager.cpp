#include "theme_manager.h"

#include "../../client/settings/cache_settings.h"

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QLibraryInfo>
#include <QMap>
#include <QMetaEnum>
#include <QPalette>
#include <QPixmapCache>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QWidget>
#include <Qt>

#define NONE_THEME_NAME "Default"
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

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    defaultStyleName = qApp->style()->objectName();
    //! \todo Workaround for windows11 style being broken.
    if (defaultStyleName == "windows11") {
        defaultStyleName = "windowsvista";
    }
    ensureThemeDirectoryExists();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &ThemeManager::themeChangedSlot);
#endif
    connect(&SettingsCache::instance(), &SettingsCache::themeChanged, this, &ThemeManager::themeChangedSlot);
    themeChangedSlot();
}

void ThemeManager::ensureThemeDirectoryExists()
{
    if (SettingsCache::instance().getThemeName().isEmpty() ||
        !getAvailableThemes().contains(SettingsCache::instance().getThemeName())) {
        qCInfo(ThemeManagerLog) << "Theme name not set, setting default value";
        SettingsCache::instance().setThemeName(NONE_THEME_NAME);
    }
}

bool ThemeManager::isDarkMode(const QString &themeDirPath, const QString &userDirPath)
{
    // User override takes precedence over system config
    for (const QString &path : {userDirPath, themeDirPath}) {
        if (path.isEmpty()) {
            continue;
        }
        ThemeConfig cfg = ThemeConfig::fromThemeDir(path);
        if (cfg.colorScheme.compare("Dark", Qt::CaseInsensitive) == 0) {
            return true;
        }
        if (cfg.colorScheme.compare("Light", Qt::CaseInsensitive) == 0) {
            return false;
        }
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
    return false;
#endif
}

bool ThemeManager::isBuiltInTheme()
{
    const auto themeName = SettingsCache::instance().getThemeName();

    return themeName == NONE_THEME_NAME || themeName == FUSION_THEME_NAME;
}

QString ThemeManager::userThemeDirFor(const QString &themeName)
{
    return QDir(SettingsCache::instance().getThemesPath()).absoluteFilePath(themeName);
}

QStringMap &ThemeManager::getAvailableThemes()
{
    availableThemes.clear();

    // ── 1. System themes (read-only, shipped with the application) ──────────
    QDir sysDir(qApp->applicationDirPath() +
#ifdef Q_OS_MAC
                "/../Resources/themes"
#elif defined(Q_OS_WIN)
                "/themes"
#else
                "/../share/cockatrice/themes"
#endif
    );
    for (const QString &name : sysDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        availableThemes.insert(name, sysDir.absoluteFilePath(name));
    }

    // ── 2. User-only themes (AppData) ────────────────────────────────────────
    // We only add themes that don't already exist in the system directory.
    // Customisations to system themes are handled via the fallthrough read
    // logic (userThemeDirFor → system path); we intentionally keep the system
    // path in the map so shipped assets are always locatable.
    QDir userDir(SettingsCache::instance().getThemesPath());
    for (const QString &name : userDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(name)) {
            availableThemes.insert(name, userDir.absoluteFilePath(name));
        }
    }

    // ── 3. Ensure built-in sentinels always exist (dev builds without install)
    if (!availableThemes.contains(NONE_THEME_NAME)) {
        availableThemes.insert(NONE_THEME_NAME, userDir.absoluteFilePath("Default"));
    }
    if (!availableThemes.contains(FUSION_THEME_NAME)) {
        availableThemes.insert(FUSION_THEME_NAME, userDir.absoluteFilePath("Fusion"));
    }

    return availableThemes;
}

ThemeConfig ThemeManager::effectiveThemeConfig(const QString &themeName)
{
    const QString dirPath = ThemeManager().getAvailableThemes().value(themeName);
    const QString userDirPath = userThemeDirFor(themeName);

    ThemeConfig userCfg = ThemeConfig::fromThemeDir(userDirPath);
    ThemeConfig systemCfg = ThemeConfig::fromThemeDir(dirPath);

    ThemeConfig result = systemCfg;

    // User values override system values on a per-field basis
    if (!userCfg.colorScheme.isEmpty()) {
        result.colorScheme = userCfg.colorScheme;
    }

    if (!userCfg.styleName.isEmpty()) {
        result.styleName = userCfg.styleName;
    }

    return result;
}

void ThemeManager::setStyleName(const QString &style)
{
    const QString themeName = SettingsCache::instance().getThemeName();
    const QString userDirPath = userThemeDirFor(themeName);

    ThemeConfig cfg = effectiveThemeConfig(themeName);

    cfg.styleName = style;
    cfg.save(userDirPath);

    reloadCurrentTheme();
}

QBrush ThemeManager::loadBrush(QString fileName, QColor fallbackColor)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:zones/" + fileName);
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
    QPixmap tmp = QPixmap("theme:zones/" + fileName);

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

void ThemeManager::setColorScheme(const QString &scheme)
{
    const QString themeName = SettingsCache::instance().getThemeName();
    const QString userDirPath = userThemeDirFor(themeName);

    ThemeConfig cfg = effectiveThemeConfig(themeName);

    cfg.colorScheme = scheme;
    cfg.save(userDirPath);

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
    const QString userDirPath = userThemeDirFor(themeName);

    ThemeConfig themeCfg = ThemeConfig::fromThemeDir(userDirPath);
    if (themeCfg.colorScheme.isEmpty() && themeCfg.styleName.isEmpty()) {
        themeCfg = ThemeConfig::fromThemeDir(dirPath);
    }

    applyStyleAndPalette(themeName, themeCfg, cfg, scheme);
}

void ThemeManager::applyStyleAndPalette(const QString &themeName,
                                        const ThemeConfig &themeCfg,
                                        const PaletteConfig &palCfg,
                                        const QString &activeScheme)
{
    QString styleName = themeCfg.styleName;
    if (styleName.isEmpty() || styleName.compare("Default", Qt::CaseInsensitive) == 0) {
        if (themeName == FUSION_THEME_NAME) {
            styleName = "Fusion";
        } else {
            styleName = defaultStyleName;
        }
    }

    QStyle *style = QStyleFactory::create(styleName);
    if (!style) {
        style = QStyleFactory::create(defaultStyleName);
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
        base = qApp->palette();
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
}

void ThemeManager::themeChangedSlot()
{
    const QString themeName = SettingsCache::instance().getThemeName();
    const QString dirPath = getAvailableThemes().value(themeName); // system path
    const QString userDirPath = userThemeDirFor(themeName);        // user override path
    currentThemePath = dirPath;

    // CSS — user override first, then system
    const auto tryLoadCss = [](const QString &path) -> bool {
        QDir d(path);
        if (!path.isEmpty() && d.exists(STYLE_CSS_NAME)) {
            qApp->setStyleSheet("file:///" + d.absoluteFilePath(STYLE_CSS_NAME));
            return true;
        }
        return false;
    };
    if (!tryLoadCss(userDirPath) && !tryLoadCss(dirPath)) {
        qApp->setStyleSheet("");
    }

    // ThemeConfig — user override first, then system
    ThemeConfig themeCfg = ThemeConfig::fromThemeDir(userDirPath);
    if (themeCfg.colorScheme.isEmpty() && themeCfg.styleName.isEmpty()) {
        themeCfg = ThemeConfig::fromThemeDir(dirPath);
    }

    const QString activeScheme = isDarkMode(dirPath, userDirPath) ? "Dark" : "Light";

    // Palette — user customisation → system palette → system default
    PaletteConfig palette = PaletteConfig::fromScheme(userDirPath, activeScheme);
    if (!palette.hasPalette()) {
        palette = PaletteConfig::fromScheme(dirPath, activeScheme);
    }
    if (!palette.hasPalette()) {
        palette = PaletteConfig::fromDefault(dirPath, activeScheme);
    }

    applyStyleAndPalette(themeName, themeCfg, palette, activeScheme);

    // Search paths — user assets shadow system assets, both fall through to builtins
    QStringList resources;
    if (QDir(userDirPath).exists()) {
        resources << userDirPath;
    }
    if (!dirPath.isEmpty()) {
        resources << dirPath;
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
