#include "thememanager.h"
#include "settingscache.h"
#include <QApplication>
#include <QDebug>
#include <QColor>
#include <QDir>
#include <QLibraryInfo>
#if QT_VERSION < 0x050000
    #include <QDesktopServices>
#else
    #include <QStandardPaths>
#endif

#define DEFAULT_THEME_NAME "Default"
#define STYLE_CSS_NAME "style.css"
#define HANDZONE_BG_NAME "handzone."
#define PLAYERZONE_BG_NAME "playerzone."
#define STACKZONE_BG_NAME "stackzone."
#define TABLEZONE_BG_NAME "tablezone."
#define CARD_BACK_NAME "cardback."

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    ensureUserThemeDirectoryExists();
    connect(settingsCache, SIGNAL(themeChanged()), this, SLOT(themeChangedSlot()));
    themeChangedSlot();
}

void ThemeManager::ensureUserThemeDirectoryExists()
{
    QString destDir =
#if QT_VERSION < 0x050000
    QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
#else
    QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
    "/themes";

    QDir tmpDir(destDir);
    if(!tmpDir.exists())
    {
        QString srcDir = QLibraryInfo::location(QLibraryInfo::DataPath) + "/themes";
        qDebug() << "Themes directory not found, copying bundled themes";
        // try to install the default images for the current user
        settingsCache->copyPath(srcDir, destDir);
    }

    if(settingsCache->getThemeName().isEmpty())
    {
        qDebug() << "Theme name not set, setting default value";
        settingsCache->setThemeName(DEFAULT_THEME_NAME);
    }
}

QStringList & ThemeManager::getAvailableThemes()
{
    QDir dir =
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
        "/themes/";

    availableThemes = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
    return availableThemes;
}

void ThemeManager::themeChangedSlot()
{
    QString themeName = settingsCache->getThemeName();
    qDebug() << "Theme changed:" << themeName;

    QDir dir =
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
        "/themes/" + themeName + "/";

    // css
    if(dir.exists(STYLE_CSS_NAME))
        qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
    else
        qApp->setStyleSheet("");

    QPixmap tmp;
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.bmp" << "*.svg";
    QStringList images = dir.entryList(filters, QDir::Files);

    // hand zone
    handBgBrush.setColor(QColor(80, 100, 50));
    handBgBrush.setStyle(Qt::SolidPattern);

    foreach (const QString &str, images) {
        if (str.startsWith(HANDZONE_BG_NAME)) {
            tmp.load(dir.absoluteFilePath(str));
            if(!tmp.isNull())
                handBgBrush.setTexture(tmp);
            break;
        }
    }

    // table zone
    tableBgBrush.setColor(QColor(70, 50, 100));
    tableBgBrush.setStyle(Qt::SolidPattern);

    foreach (const QString &str, images) {
        if (str.startsWith(TABLEZONE_BG_NAME)) {
            tmp.load(dir.absoluteFilePath(str));
            if(!tmp.isNull())
                tableBgBrush.setTexture(tmp);
            break;
        }
    }

    // player zone
    playerBgBrush.setColor(QColor(200, 200, 200));
    playerBgBrush.setStyle(Qt::SolidPattern);

    foreach (const QString &str, images) {
        if (str.startsWith(PLAYERZONE_BG_NAME)) {
            tmp.load(dir.absoluteFilePath(str));
            if(!tmp.isNull())
                playerBgBrush.setTexture(tmp);
            break;
        }
    }

    // stack zone
    stackBgBrush.setColor(QColor(113, 43, 43));
    stackBgBrush.setStyle(Qt::SolidPattern);

    foreach (const QString &str, images) {
        if (str.startsWith(STACKZONE_BG_NAME)) {
            tmp.load(dir.absoluteFilePath(str));
            if(!tmp.isNull())
                stackBgBrush.setTexture(tmp);
            break;
        }
    }

    // card background
    cardBackPixmap = QPixmap();

    foreach (const QString &str, images) {
        if (str.startsWith(CARD_BACK_NAME)) {
            cardBackPixmap.load(dir.absoluteFilePath(str));
            break;
        }
    }

    emit themeChanged();
}