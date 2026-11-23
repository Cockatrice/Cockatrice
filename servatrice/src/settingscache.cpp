#include "settingscache.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>
#include <qlogging.h>
#include <qnamespace.h>
#include <qsystemdetection.h>

class QObject;

SettingsCache::SettingsCache(const QString &fileName, QSettings::Format format, QObject *parent)
    : QSettings(fileName, format, parent)
{
    // first, figure out if we are running in portable mode
    isPortableBuild = QFile::exists(qApp->applicationDirPath() + "/portable.dat");

    rebuildDisallowedRegExps();
}

QVariant SettingsCache::value(const QString &key, const QVariant &defaultValue) const
{
    const auto it = overrides.constFind(key);
    if (it != overrides.constEnd()) {
        return it.value();
    }
    return QSettings::value(key, defaultValue);
}

void SettingsCache::setOverride(const QString &key, const QVariant &value)
{
    overrides.insert(key, value);
}

bool SettingsCache::parseOverride(const QString &entry, QString &key, QVariant &value)
{
    const int separator = entry.indexOf('=');
    if (separator <= 0) {
        return false;
    }

    const QString parsedKey = entry.left(separator).trimmed();
    if (parsedKey.isEmpty() || !parsedKey.contains('/')) {
        // A bare key is read relative to the active QSettings group by array
        // loops, so one override could rewrite every room, channel, or game type.
        return false;
    }

    const QStringList segments = parsedKey.split('/');
    if (segments.last() == "size") {
        return false;
    }
    for (const QString &segment : segments) {
        bool isNumber = false;
        segment.toInt(&isNumber);
        if (isNumber) {
            // Numeric segments identify array entries, which are read relative
            // to the active array group and never match a full-key override.
            return false;
        }
    }

    key = parsedKey;
    value = entry.mid(separator + 1);
    return true;
}

bool SettingsCache::isSecretKey(const QString &key)
{
    const QString lower = key.toLower();
    return lower.contains("password") || lower.contains("secret") || lower.contains("legacy_backup_key");
}

QStringList SettingsCache::effectiveKeys() const
{
    QStringList keys = allKeys();
    for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it) {
        if (!keys.contains(it.key())) {
            keys.append(it.key());
        }
    }
    keys.sort();
    return keys;
}

void SettingsCache::rebuildDisallowedRegExps()
{
    disallowedRegExp.clear();

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
    if (QFile::exists(guessFileName)) {
        return guessFileName;
    }

#ifdef Q_OS_UNIX
    // /etc
    guessFileName = "/etc/servatrice/" + fileName;
    if (QFile::exists(guessFileName)) {
        return guessFileName;
    }
#endif

    guessFileName = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + fileName;
    return guessFileName;
}
