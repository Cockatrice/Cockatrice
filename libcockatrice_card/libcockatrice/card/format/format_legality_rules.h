#ifndef COCKATRICE_FORMAT_LEGALITY_RULES_H
#define COCKATRICE_FORMAT_LEGALITY_RULES_H

#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>

class CardInfo;
using CardInfoPtr = QSharedPointer<CardInfo>;

struct CardCondition
{
    QString field;     // e.g. "type", "maintype", "text"
    QString matchType; // "contains", "equals", "regex", "notContains", etc.
    QString value;     // e.g. "Basic Land"
};

struct ExceptionRule
{
    QList<CardCondition> conditions; // All must match
    int maxCopies = -1;              // -1 = unlimited
};

struct FormatRules
{
    QString formatName;
    int minDeckSize = 60;
    int maxDeckSize = -1; // -1 = unlimited
    int maxSideboardSize = 15;

    int maxCopies = 4;           // default constructed rule
    int maxRestrictedCopies = 1; // Used for "restricted" legality tags

    QList<ExceptionRule> exceptions; // Cards allowed to break maxCopies
};

enum class CardMatchType
{
    Equals,
    NotEquals,
    Contains,
    NotContains,
    Regex
};

// convert string to enum
inline CardMatchType matchTypeFromString(const QString &str)
{
    if (str == "equals")
        return CardMatchType::Equals;
    if (str == "notEquals")
        return CardMatchType::NotEquals;
    if (str == "contains")
        return CardMatchType::Contains;
    if (str == "notContains")
        return CardMatchType::NotContains;
    if (str == "regex")
        return CardMatchType::Regex;
    return CardMatchType::Equals; // fallback default
}

bool cardMatchesCondition(const CardInfo &card, const CardCondition &cond);

bool exceptionAppliesToCard(const CardInfo &card, const ExceptionRule &rule);

bool cardHasAnyException(const CardInfo &card, const FormatRules &format);

#endif // COCKATRICE_FORMAT_LEGALITY_RULES_H
