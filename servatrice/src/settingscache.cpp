#include "settingscache.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

SettingsCache::SettingsCache(const QString &fileName, QSettings::Format format, QObject *parent)
    : QSettings(fileName, format, parent)
{
    // first, figure out if we are running in portable mode
    isPortableBuild = QFile::exists(qApp->applicationDirPath() + "/portable.dat");

    QStringList disallowedRegExpStr = value("users/disallowedregexp", "").toString().split(",", Qt::SkipEmptyParts);
    disallowedRegExpStr.removeDuplicates();
    for (const QString &regExpStr : disallowedRegExpStr) {
        disallowedRegExp.append(QRegularExpression(QString("\\A%1\\z").arg(regExpStr)));
    }
}

QString SettingsCache::guessConfigurationPath()
{
    const QString fileName = "servatrice.ini";
    if (QFile::exists(qApp->applicationDirPath() + "/portable.dat")) {
        qDebug() << "Portable mode enabled";
        return fileName;
    }

    QString guessFileName;

    // application directory path
    guessFileName = QCoreApplication::applicationDirPath() + "/" + fileName;
    if (QFile::exists(guessFileName))
        return guessFileName;

#ifdef Q_OS_UNIX
    // /etc
    guessFileName = "/etc/servatrice/" + fileName;
    if (QFile::exists(guessFileName))
        return guessFileName;
#endif

    guessFileName = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + fileName;
    return guessFileName;
}
