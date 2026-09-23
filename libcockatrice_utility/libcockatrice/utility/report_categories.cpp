#include "report_categories.h"

namespace ReportCategories
{
const QStringList &keys()
{
    static const QStringList categories = {"cheating",
                                           "bug_abuse",
                                           "harassment",
                                           "verbal_abuse",
                                           "hate_speech",
                                           "spam",
                                           "other"};
    return categories;
}

bool isValid(const QString &key)
{
    return keys().contains(key, Qt::CaseInsensitive);
}
} // namespace ReportCategories