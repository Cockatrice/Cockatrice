#include "translate_counter_name.h"

#include <libcockatrice/utility/counter_ids.h>

const QMap<QString, QString> TranslateCounterName::translated = {
    {"life", QT_TRANSLATE_NOOP("TranslateCounterName", "Life")},
    {"w", QT_TRANSLATE_NOOP("TranslateCounterName", "White")},
    {"u", QT_TRANSLATE_NOOP("TranslateCounterName", "Blue")},
    {"b", QT_TRANSLATE_NOOP("TranslateCounterName", "Black")},
    {"r", QT_TRANSLATE_NOOP("TranslateCounterName", "Red")},
    {"g", QT_TRANSLATE_NOOP("TranslateCounterName", "Green")},
    {"x", QT_TRANSLATE_NOOP("TranslateCounterName", "Colorless")},
    {"storm", QT_TRANSLATE_NOOP("TranslateCounterName", "Other")},
    {CounterNames::TaxCounter1, QT_TRANSLATE_NOOP("TranslateCounterName", "1st Tax")},
    {CounterNames::TaxCounter2, QT_TRANSLATE_NOOP("TranslateCounterName", "2nd Tax")},
    {CounterNames::TaxCounter3, QT_TRANSLATE_NOOP("TranslateCounterName", "3rd Tax")},
    {CounterNames::TaxCounter4, QT_TRANSLATE_NOOP("TranslateCounterName", "4th Tax")},
    {CounterNames::TaxCounter5, QT_TRANSLATE_NOOP("TranslateCounterName", "5th Tax")}};
