#include "translate_counter_name.h"

#include <libcockatrice/common/counter_ids.h>

const QMap<QString, QString> TranslateCounterName::translated = {
    {"life", QT_TRANSLATE_NOOP("TranslateCounterName", "Life")},
    {"w", QT_TRANSLATE_NOOP("TranslateCounterName", "White")},
    {"u", QT_TRANSLATE_NOOP("TranslateCounterName", "Blue")},
    {"b", QT_TRANSLATE_NOOP("TranslateCounterName", "Black")},
    {"r", QT_TRANSLATE_NOOP("TranslateCounterName", "Red")},
    {"g", QT_TRANSLATE_NOOP("TranslateCounterName", "Green")},
    {"x", QT_TRANSLATE_NOOP("TranslateCounterName", "Colorless")},
    {"storm", QT_TRANSLATE_NOOP("TranslateCounterName", "Other")},
    {CounterNames::CommanderTax, QT_TRANSLATE_NOOP("TranslateCounterName", "commander tax")},
    {CounterNames::PartnerTax, QT_TRANSLATE_NOOP("TranslateCounterName", "partner tax")}};
