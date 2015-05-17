#include "thememanager.h"
#include "settingscache.h"
#include <QApplication>
#include <QDebug>
#include <QColor>
#include <QLibraryInfo>
#if QT_VERSION < 0x050000
    #include <QDesktopServices>
#else
    #include <QStandardPaths>
#endif

#define DEFAULT_THEME_NAME "Default"
#define STYLE_CSS_NAME "style.css"
#define HANDZONE_BG_NAME "handzone"
#define PLAYERZONE_BG_NAME "playerzone"
#define STACKZONE_BG_NAME "stackzone"
#define TABLEZONE_BG_NAME "tablezone"
#define CARD_BACK_NAME "cardback"

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
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
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

QBrush ThemeManager::loadBrush(QDir dir, QString fileName, QColor fallbackColor)
{
    QBrush brush;
    QPixmap tmp;
    QStringList exts;
    exts << ".png" << ".jpg" << ".jpeg" << ".gif" << ".bmp";

    brush.setColor(fallbackColor);
    brush.setStyle(Qt::SolidPattern);

    foreach (const QString &ext, exts) {
        if (dir.exists(fileName + ext)) {
            tmp.load(dir.absoluteFilePath(fileName + ext));
            if(!tmp.isNull())
                brush.setTexture(tmp);
            break;
        }
    }

    return brush;
}

QPixmap ThemeManager::loadPixmap(QDir dir, QString fileName)
{
    QPixmap pix;
    QStringList exts;
    exts << ".png" << ".jpg" << ".jpeg" << ".gif" << ".bmp";

    foreach (const QString &ext, exts) {
        if (dir.exists(fileName + ext)) {
            pix.load(dir.absoluteFilePath(fileName + ext));
            break;
        }
    }

    return pix;
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

    // card background
    cardBackPixmap = loadPixmap(dir, CARD_BACK_NAME);

    // zones bg
    dir.cd("zones");
    handBgBrush = loadBrush(dir, HANDZONE_BG_NAME, QColor(80, 100, 50));
    tableBgBrush = loadBrush(dir, TABLEZONE_BG_NAME, QColor(70, 50, 100));
    playerBgBrush = loadBrush(dir, PLAYERZONE_BG_NAME, QColor(200, 200, 200));
    stackBgBrush = loadBrush(dir, STACKZONE_BG_NAME, QColor(113, 43, 43));

    // resources
    QStringList resources;
    resources << dir.absolutePath() << ":/resources";
    QDir::setSearchPaths("theme", resources);

    emit themeChanged();
}