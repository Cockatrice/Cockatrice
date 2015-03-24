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
#define VERSION_TXT_NAME "version.txt"
#define HANDZONE_BG_NAME "handzone"
#define PLAYERZONE_BG_NAME "playerzone"
#define STACKZONE_BG_NAME "stackzone"
#define TABLEZONE_BG_NAME "tablezone"
#define CARD_BACK_NAME "cardback"

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
    QDir::separator() + "themes";

    QString srcDir = QLibraryInfo::location(QLibraryInfo::DataPath) + QDir::separator() + "themes";
    QDir tmpDstDir(destDir);
    if(!tmpDstDir.exists())
    {
        qDebug() << "Themes directory not found, copying bundled themes";
        settingsCache->copyPath(srcDir, destDir);
    } else {
        QDir tmpSrcDir(srcDir);
        QStringList bundledThemes = tmpSrcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        // check if any bundled theme needs an update
        foreach(QString themeName, bundledThemes)
        {
            bool needCopy = false;
            QFile srcVersionFile(srcDir + QDir::separator() + themeName + QDir::separator() + VERSION_TXT_NAME);
            QFile dstVersionFile(destDir + QDir::separator() + themeName + QDir::separator() + VERSION_TXT_NAME);

            if(!srcVersionFile.exists())
            {
                qDebug() << "Corrupted installation: version.txt doesn't exists in bundled theme" << themeName;
                continue;
            }

            if(dstVersionFile.exists())
            {
                // compare version files
                srcVersionFile.open(QIODevice::ReadOnly);
                dstVersionFile.open(QIODevice::ReadOnly);
                int oldver = dstVersionFile.readAll().simplified().toUInt();
                int newver = srcVersionFile.readAll().simplified().toUInt();
                srcVersionFile.close();
                dstVersionFile.close();

                if(newver > oldver)
                {
                    needCopy = true;
                    qDebug() << "Updating bundled theme" << themeName << "from version" << oldver << "to version" << newver;
                }

            } else {
                // no version file in dest
                needCopy = true;
                qDebug() << "Installing new bundled theme" << themeName;
            }

            // copy if needed
            if(needCopy)
            {
                settingsCache->copyPath(srcDir + QDir::separator() + themeName, destDir + QDir::separator() + themeName);
            }
        }
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
        QDir::separator() + "themes";

    availableThemes = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
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

    QDir dir =
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first() +
#endif
        QDir::separator() + "themes" + QDir::separator() + themeName + QDir::separator();

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