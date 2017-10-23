#include "thememanager.h"
#include "settingscache.h"
#include <QApplication>
#include <QDebug>
#include <QColor>
#include <QPixmapCache>
#include <QLibraryInfo>
#include <QStandardPaths>

#define DEFAULT_THEME_NAME "Default"
#define STYLE_CSS_NAME "style.css"
#define HANDZONE_BG_NAME "handzone"
#define PLAYERZONE_BG_NAME "playerzone"
#define STACKZONE_BG_NAME "stackzone"
#define TABLEZONE_BG_NAME "tablezone"

ThemeManager::ThemeManager(QObject *parent)
    :QObject(parent)
{
    ensureThemeDirectoryExists();
    connect(settingsCache, SIGNAL(themeChanged()), this, SLOT(themeChangedSlot()));
    themeChangedSlot();
}

void ThemeManager::ensureThemeDirectoryExists()
{
    if(settingsCache->getThemeName().isEmpty() ||
        !getAvailableThemes().contains(settingsCache->getThemeName()))
    {
        qDebug() << "Theme name not set, setting default value";
        settingsCache->setThemeName(DEFAULT_THEME_NAME);
    }
}

QStringMap & ThemeManager::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // load themes from user profile dir
    dir =
#ifdef PORTABLE_BUILD
        qApp->applicationDirPath() +
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
        "/themes";

    foreach(QString themeName, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if(!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    // load themes from cockatrice system dir
#ifdef Q_OS_MAC
    dir = qApp->applicationDirPath() + "/../Resources/themes";
#elif defined(Q_OS_WIN)
    dir = qApp->applicationDirPath() + "/themes";
#else // linux
    dir = qApp->applicationDirPath() + "/../share/cockatrice/themes";
#endif
    foreach(QString themeName, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if(!availableThemes.contains(themeName))
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
    }

    return availableThemes;
}

QBrush ThemeManager::loadBrush(QString fileName, QColor fallbackColor)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:zones/" + fileName);
    if(tmp.isNull())
    {
        brush.setColor(fallbackColor);
        brush.setStyle(Qt::SolidPattern);
    } else {
        brush.setTexture(tmp);
    }

    return brush;
}

void ThemeManager::themeChangedSlot()
{
    QString themeName = settingsCache->getThemeName();
    qDebug() << "Theme changed:" << themeName;

    QDir dir = getAvailableThemes().value(themeName);

    // css
    if(dir.exists(STYLE_CSS_NAME))

        qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
    else
        qApp->setStyleSheet("");

    // resources
    QStringList resources;
    resources << dir.absolutePath() << ":/resources";
    QDir::setSearchPaths("theme", resources);

    // zones bg
    dir.cd("zones");
    handBgBrush = loadBrush(HANDZONE_BG_NAME, QColor(80, 100, 50));
    tableBgBrush = loadBrush(TABLEZONE_BG_NAME, QColor(70, 50, 100));
    playerBgBrush = loadBrush(PLAYERZONE_BG_NAME, QColor(200, 200, 200));
    stackBgBrush = loadBrush(STACKZONE_BG_NAME, QColor(113, 43, 43));

    QPixmapCache::clear();

    emit themeChanged();
}
