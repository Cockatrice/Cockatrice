#ifndef WARNING_CATEGORIES_H
#define WARNING_CATEGORIES_H

#include <QList>
#include <QString>

/**
 * A warning category the server offers to moderators, optionally carrying a
 * recommended starting intervention level (see the moderator guide).
 */
struct WarningCategory
{
    QString name;
    int startingIl = 1;
};

/**
 * Parses the `server/officialwarnings` setting value into warning categories.
 *
 * Entries are separated by commas. Each entry is a category name, optionally
 * followed by "|" and the recommended starting intervention level:
 *   "Abusive Language|1,Cheating|2,Spamming"
 * Entries without an explicit level default to intervention level 1.
 * Empty entries are skipped.
 */
QList<WarningCategory> parseWarningCategories(const QString &value);

#endif // WARNING_CATEGORIES_H
