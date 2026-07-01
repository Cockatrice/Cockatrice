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
#define COMMANDZONE_BG_NAME "commandzone"
static const QColor HANDZONE_BG_DEFAULT = QColor(80, 100, 50);
static const QColor TABLEZONE_BG_DEFAULT = QColor(70, 50, 100);
static const QColor PLAYERZONE_BG_DEFAULT = QColor(200, 200, 200);
static const QColor STACKZONE_BG_DEFAULT = QColor(113, 43, 43);
static const QColor COMMANDZONE_BG_DEFAULT = QColor(50, 60, 80);
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

bool ThemeManager::isDarkMode(const QString &themeDirPath)
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

bool ThemeManager::isBuiltInTheme()
{
    const auto themeName = SettingsCache::instance().getThemeName();

    return themeName == NONE_THEME_NAME || themeName == FUSION_THEME_NAME;
}

QStringMap &ThemeManager::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // load themes from user profile dir
    dir.setPath(SettingsCache::instance().getThemesPath());

    // add default value
    availableThemes.insert(NONE_THEME_NAME, dir.absoluteFilePath("Default"));

    availableThemes.insert(FUSION_THEME_NAME, dir.absoluteFilePath("Fusion"));

    for (QString themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName)) {
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
        }
    }

    // load themes from cockatrice system dir
    dir.setPath(qApp->applicationDirPath() +
#ifdef Q_OS_MAC
                "/../Resources/themes"
#elif defined(Q_OS_WIN)
                "/themes"
#else // linux
                "/../share/cockatrice/themes"
#endif
    );

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
    const QString dirPath = getAvailableThemes().value(SettingsCache::instance().getThemeName());
    ThemeConfig cfg = ThemeConfig::fromThemeDir(dirPath);

    cfg.colorScheme = scheme;

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
    QString themeName = SettingsCache::instance().getThemeName();
    QString dirPath = getAvailableThemes().value(themeName);
    currentThemePath = dirPath;
    QDir dir(dirPath);

    // CSS
    if (!dirPath.isEmpty() && dir.exists(STYLE_CSS_NAME)) {
        qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
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
    if (!palette.hasPalette()) {
        palette = PaletteConfig::fromDefault(dirPath, activeScheme);
    }

    applyStyleAndPalette(themeName, themeCfg, palette, activeScheme);

    QStringList resources;
    if (!dirPath.isEmpty()) {
        resources << dir.absolutePath();
    }
    resources << DEFAULT_RESOURCE_PATHS;

    QDir::setSearchPaths("theme", resources);

    brushes[Role::Hand] = loadBrush(HANDZONE_BG_NAME, HANDZONE_BG_DEFAULT);

    brushes[Role::Table] = loadBrush(TABLEZONE_BG_NAME, TABLEZONE_BG_DEFAULT);

    brushes[Role::Player] = loadBrush(PLAYERZONE_BG_NAME, PLAYERZONE_BG_DEFAULT);

    brushes[Role::Stack] = loadBrush(STACKZONE_BG_NAME, STACKZONE_BG_DEFAULT);

    brushes[Role::Command] = loadBrush(COMMANDZONE_BG_NAME, COMMANDZONE_BG_DEFAULT);
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

        case ThemeManager::Command:
            return COMMANDZONE_BG_NAME;

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
