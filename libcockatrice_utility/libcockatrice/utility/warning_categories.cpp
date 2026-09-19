#include "warning_categories.h"

QList<WarningCategory> parseWarningCategories(const QString &value)
{
    QList<WarningCategory> categories;
    const QStringList entries = value.split(',', Qt::SkipEmptyParts);
    for (const QString &entry : entries) {
        const QStringList parts = entry.split('|');
        WarningCategory category;
        category.name = parts.first().trimmed();
        if (category.name.isEmpty()) {
            continue;
        }
        if (parts.size() > 1) {
            bool ok = false;
            const int il = parts.at(1).trimmed().toInt(&ok);
            if (ok && il > 0) {
                category.startingIl = il;
            }
        }
        categories.append(category);
    }
    return categories;
}
