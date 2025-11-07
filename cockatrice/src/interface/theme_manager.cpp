#include "theme_manager.h"

#include "../../client/settings/cache_settings.h"

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QLibraryInfo>
#include <QPixmapCache>
#include <QStandardPaths>

#define NONE_THEME_NAME "Default"
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

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    ensureThemeDirectoryExists();
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

QStringMap &ThemeManager::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // add default value
    availableThemes.insert(NONE_THEME_NAME, QString());

    // load themes from user profile dir
    dir.setPath(SettingsCache::instance().getThemesPath());

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

void ThemeManager::themeChangedSlot()
{
    QString themeName = SettingsCache::instance().getThemeName();
    qCInfo(ThemeManagerLog) << "Theme changed:" << themeName;

    QString dirPath = getAvailableThemes().value(themeName);
    QDir dir = dirPath;

    // css
    if (!dirPath.isEmpty() && dir.exists(STYLE_CSS_NAME)) {
        qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
    } else {
        qApp->setStyleSheet("");
    }

    if (dirPath.isEmpty()) {
        // set default values
        QDir::setSearchPaths("theme", DEFAULT_RESOURCE_PATHS);
        brushes[Role::Hand] = HANDZONE_BG_DEFAULT;
        brushes[Role::Table] = TABLEZONE_BG_DEFAULT;
        brushes[Role::Player] = PLAYERZONE_BG_DEFAULT;
        brushes[Role::Stack] = STACKZONE_BG_DEFAULT;
    } else {
        // resources
        QStringList resources;
        resources << dir.absolutePath() << DEFAULT_RESOURCE_PATHS;
        QDir::setSearchPaths("theme", resources);

        // zones bg
        dir.cd("zones");
        brushes[Role::Hand] = loadBrush(HANDZONE_BG_NAME, HANDZONE_BG_DEFAULT);
        brushes[Role::Table] = loadBrush(TABLEZONE_BG_NAME, TABLEZONE_BG_DEFAULT);
        brushes[Role::Player] = loadBrush(PLAYERZONE_BG_NAME, PLAYERZONE_BG_DEFAULT);
        brushes[Role::Stack] = loadBrush(STACKZONE_BG_NAME, STACKZONE_BG_DEFAULT);
    }
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
