#ifndef NATURAL_SORT_COLLATOR_H
#define NATURAL_SORT_COLLATOR_H

#include <QCollator>
#include <QLocale>

/**
 * @brief A collator that compares runs of digits numerically, whatever the process locale is.
 *
 * Numeric mode only takes effect through a real locale. Under the C locale, which
 * headless CI processes run in, QCollator silently falls back to plain string order
 * and sorts "Forest 10" before "Forest 2". Pin English for that single degenerate
 * locale so sorting is the same everywhere, and keep the user's locale otherwise.
 */
inline QCollator makeNaturalSortCollator()
{
    QCollator collator;
    collator.setNumericMode(true);
    if (QLocale().language() == QLocale::C) {
        collator.setLocale(QLocale(QLocale::English));
    }
    return collator;
}

#endif // NATURAL_SORT_COLLATOR_H
