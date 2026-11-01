#include "parsehelpers.h"

#include <QChar>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>

/**
 * Parses the card text to determine if the card should have the cipt tag
 *
 * The parsing logic is able to handle the following cases:
 * - "<name> enters tapped"
 * - "<shortname> enters tapped", if the card name starts with the shortname
 * - "This <type> enters tapped"
 * - "..., it enters tapped"
 * - Any naming scheme that appends a non-alphanumeric character plus extra text to the end of the name.
 * (e.g. name is "Card Name_SET" or "Card Name (Set)" and text contains "Card Name enters tapped")
 *
 * However, it will still miss on certain cases:
 * - shortnames that aren't the at the beginning of the card name
 *
 * Note that "...enters tapped unless..." returns false.
 *
 * @param name The name of the card
 * @param text The Oracle text of the card
 */
bool parseCipt(const QString &name, const QString &text)
{
    // Use precompiled regex to check if text is a possible candidate, and early return if not
    static auto prelimCheck = QRegularExpression(" enters( the battlefield)? tapped(?! unless)");
    if (!prelimCheck.match(text).hasMatch()) {
        return false;
    }

    // Try to split shortname on most non-alphanumeric characters (including _)
    auto isShortnameDivider = [](const QChar &c) {
        return c == '_' || (!c.isLetterOrNumber() && c != '\'' && c != '\"');
    };

    // Try all possible shortnames.
    // This also handles the case of extra text appended at end.
    QStringList possibleNames;
    bool inAlphanumericPart = true;
    for (int i = 0; i < name.length(); ++i) {
        if (isShortnameDivider(name.at(i))) {
            if (inAlphanumericPart) {
                // only add to names on a "falling edge", in order to reduce the amount of redundant splits
                possibleNames.append(QRegularExpression::escape(name.left(i)));
                inAlphanumericPart = false;
            }
        } else {
            inAlphanumericPart = true;
        }
    }

    // and the full name
    possibleNames.append(QRegularExpression::escape(name));

    QString subject = "(it|"            // "..., it enters tapped"
                      "(T|t)his [^ ]+|" // "This <type> enters tapped"
                      + possibleNames.join("|") + ")";

    auto ciptPattern = QRegularExpression(
        // cipt phrase is either first sentence of line, or is after a punctuation mark
        "(^|(, |\\. ))" + subject +
            // support old wording, and exclude the "unless" case
            " enters( the battlefield)? tapped(?! unless)",
        QRegularExpression::MultilineOption);

    return ciptPattern.match(text).hasMatch();
}

std::optional<int> parseDeckCopyLimit(const QString &name, const QString &text)
{
    // Require a complete, standalone allowance. Other effects that mention a
    // number of cards or a different card name must not change deck construction.
    static const QRegularExpression rule(
        R"(^A deck can have up to ([a-z]+(?:[ -][a-z]+)*|[0-9]+) cards? named ([^\r\n]+)\.\r?$)",
        QRegularExpression::MultilineOption);
    static const QStringList smallNumbers = {
        "zero", "one",    "two",    "three",    "four",     "five",    "six",     "seven",     "eight",    "nine",
        "ten",  "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
    static const QStringList tens = {"twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
    auto normalizedName = [](QString cardName) {
        return cardName.replace("Æ", "AE").replace("’", "'").normalized(QString::NormalizationForm_C);
    };
    const QString expectedName = normalizedName(name);
    if (expectedName.isEmpty()) {
        return std::nullopt;
    }
    std::optional<int> limit;
    auto matches = rule.globalMatch(text);
    while (matches.hasNext()) {
        const auto match = matches.next();
        if (normalizedName(match.captured(2)) != expectedName) {
            continue;
        }
        const QString count = match.captured(1);
        bool numeric = false;
        int value = count.toInt(&numeric);
        if (!numeric) {
            // English words through ninety-nine; larger counts remain supported
            // as decimal numerals. Unknown wording is deliberately not guessed.
            const QStringList words = QString(count).replace('-', ' ').split(' ');
            value = words.size() == 1 ? smallNumbers.indexOf(words.first()) : -1;
            const int ten = tens.indexOf(words.first());
            if (ten >= 0 && words.size() <= 2) {
                const int unit = words.size() == 1 ? 0 : smallNumbers.indexOf(words.last());
                if (unit >= 0 && unit < 10 && (words.size() == 1 || unit > 0)) {
                    value = (ten + 2) * 10 + unit;
                }
            }
        }
        if (value <= 0 || (limit && *limit != value)) {
            return std::nullopt;
        }
        limit = value;
    }
    return limit;
}
