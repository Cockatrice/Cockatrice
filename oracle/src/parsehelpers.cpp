#include "parsehelpers.h"

#include <QRegularExpression>

/**
 * Parses the card text to determine if the card enters the battlefield tapped.
 *
 * The parsing logic will be able to handle the following cases:
 * - "<name> enters tapped..."
 * - shortname, if it appears at the start of the name.
 * - "This <type> enters tapped..."
 * - Any naming scheme that appends a non-alphanumeric character plus extra text to the end of the name.
 * (e.g. name is "Card Name_SET" or "Card Name (Set)" and text contains "Card Name enters tapped...")
 *
 * However, it will still miss certain cases:
 * - shortnames that aren't the first word in the card name
 *
 * @param name The name of the card
 * @param text The oracle text of the card
 */
bool parseCipt(const QString &name, const QString &text)
{
    static auto WHITESPACE_REGEX = QRegularExpression("\\s+");
    QString shortname = name.split(WHITESPACE_REGEX, Qt::SkipEmptyParts).first();

    QString namePattern = QString("( it|this [^ ]* |%1|%2.*)")
                              .arg(QRegularExpression::escape(name), QRegularExpression::escape(shortname));

    QRegularExpression ciptRegex(namePattern + " enters( the battlefield)? tapped(?! unless)");

    return ciptRegex.match(text).hasMatch();
}
