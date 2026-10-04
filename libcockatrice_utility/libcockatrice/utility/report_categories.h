#ifndef REPORT_CATEGORIES_H
#define REPORT_CATEGORIES_H

#include <QString>
#include <QStringList>

/**
 * Canonical report categories, shared by the client and the server.
 *
 * This is the single source of truth for the category keys exchanged over the
 * wire (Command_Report.category) and stored in the reports table. The server
 * only accepts keys from this list and the client report dialog only offers
 * keys from this list, so the two sides cannot drift apart again.
 */
namespace ReportCategories
{
/** @brief The complete list of valid category keys, in canonical order. */
const QStringList &keys();

/** @brief Whether @a key is a valid report category (case-insensitive). */
bool isValid(const QString &key);
} // namespace ReportCategories

#endif // REPORT_CATEGORIES_H