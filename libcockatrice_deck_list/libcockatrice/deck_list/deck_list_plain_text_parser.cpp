#include "deck_list_plain_text_parser.h"

#include "deck_list_node_tree.h"
#include "libcockatrice/deck_list/deck_list.h"
#include "tree/inner_deck_list_node.h"

#include <QChar>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QTextStream>
#include <QtGlobal>
#include <qnamespace.h>

namespace DeckListPlainText
{

bool parse(QTextStream &in,
           const std::function<QString(const QString &)> &cardNameNormalizer,
           DeckList::Metadata &metadata,
           DecklistNodeTree &tree)
{
    tree.clear();

    static const QRegularExpression reCardLine(R"(^\s*[\w\[\(\{].*$)", QRegularExpression::UseUnicodePropertiesOption);
    static const QRegularExpression reEmpty("^\\s*$");
    static const QRegularExpression reComment(R"([\w\[\(\{].*$)", QRegularExpression::UseUnicodePropertiesOption);
    static const QRegularExpression reSBMark("^\\s*sb:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reSBComment("^sideboard\\b.*$", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reDeckComment("^((main)?deck(list)?|mainboard)\\b",
                                                  QRegularExpression::CaseInsensitiveOption);

    // Regex for advanced card parsing
    static const QRegularExpression reMultiplier(R"(^[xX\(\[]*(\d+)[xX\*\)\]]* ?(.+))");
    static const QRegularExpression reTabQuantity(R"(^\d+[xX]?$)");

    // Regex for extracting set code and collector number with attached symbols
    static const QRegularExpression reHyphenFormat(R"(\((\w{3,})\)\s+(\w{3,})-(\d+[^\w\s]*))");
    static const QRegularExpression reRegularFormat(R"(\((\w{3,})\)\s+(\d+[^\w\s]*))");

    auto inputs = in.readAll().trimmed().split('\n');
    auto max_line = inputs.size();

    // Start at the first empty line before the first card line
    auto deckStart = inputs.indexOf(reCardLine);
    if (deckStart == -1) {
        if (inputs.indexOf(reComment) == -1) {
            return false; // Input is empty
        }
        deckStart = max_line;
    } else {
        deckStart = inputs.lastIndexOf(reEmpty, deckStart);
        if (deckStart == -1) {
            deckStart = 0;
        }
    }

    // find sideboard position, if marks are used this won't be needed
    int sBStart = -1;
    if (inputs.indexOf(reSBMark, deckStart) == -1) {
        sBStart = inputs.indexOf(reSBComment, deckStart);
        if (sBStart == -1) {
            sBStart = inputs.indexOf(reEmpty, deckStart + 1);
            if (sBStart == -1) {
                sBStart = max_line;
            }
            auto nextCard = inputs.indexOf(reCardLine, sBStart + 1);
            if (inputs.indexOf(reEmpty, nextCard + 1) != -1) {
                sBStart = max_line;
            }
        }
    }

    int index = 0;
    QRegularExpressionMatch match;

    // Parse name and comments
    while (index < deckStart) {
        const auto &current = inputs.at(index++);
        if (!current.contains(reEmpty)) {
            match = reComment.match(current);
            metadata.name = match.captured();
            break;
        }
    }
    while (index < deckStart) {
        const auto &current = inputs.at(index++);
        if (!current.contains(reEmpty)) {
            match = reComment.match(current);
            metadata.comments += match.captured() + '\n';
        }
    }
    metadata.comments.chop(1);

    // Discard empty lines
    while (index < max_line && inputs.at(index).contains(reEmpty)) {
        ++index;
    }

    // Discard line if it starts with deck or mainboard, all cards until the sideboard starts are in the mainboard
    if (inputs.at(index).contains(reDeckComment)) {
        ++index;
    }

    // Parse decklist
    for (; index < max_line; ++index) {
        // check if line is a card
        match = reCardLine.match(inputs.at(index));
        if (!match.hasMatch()) {
            continue;
        }

        const QString rawLine = match.captured();
        QString cardName = rawLine.simplified();
        bool sideboard = false;
        int amount = 1;

        // Card names never contain tab characters, so a leading numeric field
        // marks a tabular record emitted by third-party deck tools (e.g. Delver
        // Lens "2\tCard Name\tSet Name"). The first two columns are the quantity
        // and the card name; any further columns are printer/set metadata and
        // are discarded.
        bool hasTabQuantity = false;
        const auto tabFields = rawLine.split('\t');
        int firstField = 0;
        while (firstField < tabFields.size() && tabFields.at(firstField).trimmed().isEmpty()) {
            ++firstField;
        }
        if (firstField + 1 < tabFields.size()) {
            const QString quantity = tabFields.at(firstField).trimmed();
            if (reTabQuantity.match(quantity).hasMatch()) {
                hasTabQuantity = true;
                QString digits = quantity;
                if (digits.endsWith('x') || digits.endsWith('X')) {
                    digits.chop(1);
                }
                amount = digits.toInt();
                cardName = tabFields.at(firstField + 1).simplified();
            }
        }

        // Sideboard detection
        if (sBStart < 0) {
            match = reSBMark.match(cardName);
            if (match.hasMatch()) {
                sideboard = true;
                cardName = match.captured(1);
            }
        } else {
            if (index == sBStart) {
                continue;
            }
            sideboard = index > sBStart;
        }

        // Extract set code, collector number, and foil
        QString setCode;
        QString collectorNumber;
        bool isFoil = false;

        // Check for foil status at the end of the card name
        if (cardName.endsWith("*F*", Qt::CaseInsensitive)) {
            isFoil = true;
            cardName.chop(3); // Remove the "*F*" from the card name
        }
        Q_UNUSED(isFoil);

        // Attempt to match the hyphen-separated format (PLST-2094)
        match = reHyphenFormat.match(cardName);
        if (match.hasMatch()) {
            setCode = match.captured(2).toUpper();
            collectorNumber = match.captured(3);
            cardName = cardName.left(match.capturedStart()).trimmed();
        } else {
            // Attempt to match the regular format (PLST) 2094
            match = reRegularFormat.match(cardName);
            if (match.hasMatch()) {
                setCode = match.captured(1).toUpper();
                collectorNumber = match.captured(2);
                cardName = cardName.left(match.capturedStart()).trimmed();
            }
        }

        // check if a specific amount is mentioned
        if (!hasTabQuantity) {
            match = reMultiplier.match(cardName);
            if (match.hasMatch()) {
                amount = match.captured(1).toInt();
                cardName = match.captured(2);
            }
        }

        // Normalize the card name
        cardName = cardNameNormalizer(cardName);

        // Determine the zone (mainboard/sideboard)
        QString zoneName = sideboard ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

        // make new entry in decklist
        tree.addCard(cardName, amount, zoneName, -1, setCode, collectorNumber);
    }

    return true;
}

} // namespace DeckListPlainText