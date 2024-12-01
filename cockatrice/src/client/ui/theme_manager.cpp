#include "theme_manager.h"

#include "../../settings/cache_settings.h"

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
    connect(&SettingsCache::instance(), SIGNAL(themeChanged()), this, SLOT(themeChangedSlot()));
    themeChangedSlot();
}

void ThemeManager::ensureThemeDirectoryExists()
{
    if (SettingsCache::instance().getThemeName().isEmpty() ||
        !getAvailableThemes().contains(SettingsCache::instance().getThemeName())) {
        qDebug() << "Theme name not set, setting default value";
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
    qDebug() << "Theme changed:" << themeName;

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
        handBgBrush = HANDZONE_BG_DEFAULT;
        tableBgBrush = TABLEZONE_BG_DEFAULT;
        playerBgBrush = PLAYERZONE_BG_DEFAULT;
        stackBgBrush = STACKZONE_BG_DEFAULT;
    } else {
        // resources
        QStringList resources;
        resources << dir.absolutePath() << DEFAULT_RESOURCE_PATHS;
        QDir::setSearchPaths("theme", resources);

        // zones bg
        dir.cd("zones");
        handBgBrush = loadBrush(HANDZONE_BG_NAME, HANDZONE_BG_DEFAULT);
        tableBgBrush = loadBrush(TABLEZONE_BG_NAME, TABLEZONE_BG_DEFAULT);
        playerBgBrush = loadBrush(PLAYERZONE_BG_NAME, PLAYERZONE_BG_DEFAULT);
        stackBgBrush = loadBrush(STACKZONE_BG_NAME, STACKZONE_BG_DEFAULT);
    }
    tableBgBrushesCache.clear();
    stackBgBrushesCache.clear();
    playerBgBrushesCache.clear();
    handBgBrushesCache.clear();

    QPixmapCache::clear();

    emit themeChanged();
}

QBrush ThemeManager::getExtraTableBgBrush(QString extraNumber, QBrush &fallbackBrush)
{
    QBrush returnBrush;

    if (!tableBgBrushesCache.contains(extraNumber.toInt())) {
        returnBrush = loadExtraBrush(TABLEZONE_BG_NAME + extraNumber, fallbackBrush);
        tableBgBrushesCache.insert(extraNumber.toInt(), returnBrush);
    } else {
        returnBrush = tableBgBrushesCache.value(extraNumber.toInt());
    }

    return returnBrush;
}

QBrush ThemeManager::getExtraStackBgBrush(QString extraNumber, QBrush &fallbackBrush)
{
    QBrush returnBrush;

    if (!stackBgBrushesCache.contains(extraNumber.toInt())) {
        returnBrush = loadExtraBrush(STACKZONE_BG_NAME + extraNumber, fallbackBrush);
        stackBgBrushesCache.insert(extraNumber.toInt(), returnBrush);
    } else {
        returnBrush = stackBgBrushesCache.value(extraNumber.toInt());
    }

    return returnBrush;
}

QBrush ThemeManager::getExtraPlayerBgBrush(QString extraNumber, QBrush &fallbackBrush)
{
    QBrush returnBrush;

    if (!playerBgBrushesCache.contains(extraNumber.toInt())) {
        returnBrush = loadExtraBrush(PLAYERZONE_BG_NAME + extraNumber, fallbackBrush);
        playerBgBrushesCache.insert(extraNumber.toInt(), returnBrush);
    } else {
        returnBrush = playerBgBrushesCache.value(extraNumber.toInt());
    }

    return returnBrush;
}

QBrush ThemeManager::getExtraHandBgBrush(QString extraNumber, QBrush &fallbackBrush)
{
    QBrush returnBrush;

    if (!handBgBrushesCache.contains(extraNumber.toInt())) {
        returnBrush = loadExtraBrush(HANDZONE_BG_NAME + extraNumber, fallbackBrush);
        handBgBrushesCache.insert(extraNumber.toInt(), returnBrush);
    } else {
        returnBrush = handBgBrushesCache.value(extraNumber.toInt());
    }

    return returnBrush;
}
