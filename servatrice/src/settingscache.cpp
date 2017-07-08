#include "settingscache.h"
#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

SettingsCache::SettingsCache(const QString & fileName, QSettings::Format format, QObject * parent)
    :QSettings(fileName, format, parent)
{
    // first, figure out if we are running in portable mode
    isPortableBuild = QFile::exists(qApp->applicationDirPath() + "/portable.dat");

    QStringList disallowedRegExpStr = value("users/disallowedregexp", "").toString().split(",", QString::SkipEmptyParts);
    disallowedRegExpStr.removeDuplicates();
    for (const QString &regExpStr : disallowedRegExpStr) {
        disallowedRegExp.append(QRegExp(regExpStr));
    }
}

QString SettingsCache::guessConfigurationPath(QString & specificPath)
{
    const QString fileName="servatrice.ini";
    if(QFile::exists(qApp->applicationDirPath() + "/portable.dat"))
    {
        qDebug() << "Portable mode enabled";
        return fileName;
    }

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

    guessFileName = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + fileName;
    return guessFileName;
}
