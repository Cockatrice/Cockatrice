#include "parsehelpers.h"

#include <QRegularExpression>

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
 * @param text The oracle text of the card
 */
bool parseCipt(const QString &name, const QString &text)
{
    // try to split shortname on all non-alphanumeric characters (including _)
    static auto SHORTNAME_DIVIDERS = QRegularExpression("[^\\w]|_");

    // Try all possible shortnames.
    // This also handles the case of extra text appended at end.
    QStringList possibleNames;
    qsizetype i = 0;
    while ((i = name.indexOf(SHORTNAME_DIVIDERS, i)) != -1) {
        possibleNames.append(QRegularExpression::escape(name.first(i)));
        ++i;
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
