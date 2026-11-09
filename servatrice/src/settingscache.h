#ifndef SERVATRICE_SETTINGSCACHE_H
#define SERVATRICE_SETTINGSCACHE_H

#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <qtmetamacros.h>

class QObject;

class SettingsCache : public QSettings
{
    Q_OBJECT
private:
    bool isPortableBuild;
    QHash<QString, QVariant> overrides;

public:
    SettingsCache(const QString &fileName = "servatrice.ini",
                  QSettings::Format format = QSettings::IniFormat,
                  QObject *parent = 0);
    static QString guessConfigurationPath();

    /**
     * @brief Read a value, preferring a per-run override set with setOverride().
     *
     * QSettings::value() is not virtual, so this hides the base member. Every
     * server call site reads through a SettingsCache pointer, which is why the
     * override is visible without changing those call sites.
     */
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /** @brief Override a key for this process only; never written back to the ini file. */
    void setOverride(const QString &key, const QVariant &value);

    /**
     * @brief Parse one --set entry into a key and value.
     *
     * Rejects a missing '=', an empty key, and array-scoped keys (a numeric
     * path segment or a trailing "/size"). QSettings reads array entries
     * through beginReadArray(), which bypasses this class's value(), so an
     * override for those keys would be silently ignored.
     */
    static bool parseOverride(const QString &entry, QString &key, QVariant &value);

    /** @brief True when --print-config must redact this key's value. */
    static bool isSecretKey(const QString &key);

    /** @brief allKeys() plus override-only keys, sorted; the --print-config listing. */
    QStringList effectiveKeys() const;

    /** @brief Overrides in effect, for --print-config and tests. */
    const QHash<QString, QVariant> &getOverrides() const
    {
        return overrides;
    }

    /** @brief Rebuild disallowedRegExp from users/disallowedregexp after overrides are applied. */
    void rebuildDisallowedRegExps();

    QList<QRegularExpression> disallowedRegExp;
    bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }
};

extern SettingsCache *settingsCache;

#endif
