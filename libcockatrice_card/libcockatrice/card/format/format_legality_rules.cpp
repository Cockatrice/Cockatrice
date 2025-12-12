#include "format_legality_rules.h"

#include <libcockatrice/card/card_info.h>

bool cardMatchesCondition(const CardInfo &card, const CardCondition &cond)
{
    CardMatchType type = matchTypeFromString(cond.matchType);
    QString fieldValue;
    if (cond.field == "name") {
        fieldValue = card.getName();
    } else if (cond.field == "text") {
        fieldValue = card.getText();
    } else {
        fieldValue = card.getProperty(cond.field);
    }

    switch (type) {
        case CardMatchType::Equals:
            return fieldValue == cond.value;
        case CardMatchType::NotEquals:
            return fieldValue != cond.value;
        case CardMatchType::Contains:
            return fieldValue.contains(cond.value, Qt::CaseInsensitive);
        case CardMatchType::NotContains:
            return !fieldValue.contains(cond.value, Qt::CaseInsensitive);
        case CardMatchType::Regex: {
            QRegularExpression re(cond.value, QRegularExpression::CaseInsensitiveOption);
            return re.match(fieldValue).hasMatch();
        }
        default:
            return false;
    }
}

bool exceptionAppliesToCard(const CardInfo &card, const ExceptionRule &rule)
{
    for (const CardCondition &cond : rule.conditions) {
        if (!cardMatchesCondition(card, cond)) {
            return false; // all conditions must match
        }
    }
    return true;
}

bool cardHasAnyException(const CardInfo &card, const FormatRules &format)
{
    for (const ExceptionRule &rule : format.exceptions) {
        if (exceptionAppliesToCard(card, rule)) {
            return true;
        }
    }
    return false;
}