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
    // Try to split shortname on most non-alphanumeric characters (including _)
    static QSet<QChar> exceptions = {'\'', '\"'};
    static auto isShortnameDivider = [](const QChar &c) {
        return c == '_' || (!c.isLetterOrNumber() && !exceptions.contains(c));
    };

    // Try all possible shortnames.
    // This also handles the case of extra text appended at end.
    QStringList possibleNames;
    bool inAlphanumericPart = true;
    for (int i = 0; i < name.length(); ++i) {
        if (isShortnameDivider(name.at(i))) {
            if (inAlphanumericPart) {
                // only add to names on a "falling edge", in order to reduce the amount of redundant splits
                possibleNames.append(QRegularExpression::escape(name.first(i)));
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
