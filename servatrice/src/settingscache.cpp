#include "settingscache.h"
#include <QCoreApplication>
#include <QFile>
#if QT_VERSION >= 0x050000
    #include <QStandardPaths>
#else
    #include <QDesktopServices>
#endif

SettingsCache::SettingsCache(const QString & fileName, QSettings::Format format, QObject * parent)
:QSettings(fileName, format, parent)
{

}

QString SettingsCache::guessConfigurationPath(QString & specificPath)
{
    const QString fileName="servatrice.ini";    
    #ifdef PORTABLE_BUILD
    return fileName;
    #endif
    QString guessFileName;
    // specific path
    if(!specificPath.isEmpty() && QFile::exists(specificPath))
        return specificPath;

    // application directory path
    guessFileName = QCoreApplication::applicationDirPath() + "/" + fileName;
    if(QFile::exists(guessFileName))
        return guessFileName;

#ifdef Q_OS_UNIX
    // /etc
    guessFileName = "/etc/servatrice/" + fileName;
    if(QFile::exists(guessFileName))
        return guessFileName;
#endif

#if QT_VERSION >= 0x050000
    guessFileName =  QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + fileName;
#else
    guessFileName =  QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + fileName;
#endif
    return guessFileName;
}
