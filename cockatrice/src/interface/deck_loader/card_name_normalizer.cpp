#include "card_name_normalizer.h"

#include "libcockatrice/card/database/card_database_manager.h"

QString CardNameNormalizer::operator()(const QString &cardNameString) const
{
    QString cardName = cardNameString;

    // Regex for advanced card parsing
    static const QRegularExpression reSplitCard(R"( ?\/\/ ?)");
    static const QRegularExpression reBrace(R"( ?[\[\{][^\]\}]*[\]\}] ?)"); // not nested
    static const QRegularExpression reRoundBrace(R"(^\([^\)]*\) ?)");       // () are only matched at start of string
    static const QRegularExpression reDigitBrace(R"( ?\(\d*\) ?)");         // () are matched if containing digits
    static const QRegularExpression reBraceDigit(
        R"( ?\([\dA-Z]+\) *\d+$)"); // () are matched if containing setcode then a number
    static const QRegularExpression reDoubleFacedMarker(R"( ?\(Transform\) ?)");

    static const QHash<QRegularExpression, QString> differences{{QRegularExpression("’"), "'"},
                                                                {QRegularExpression("Æ"), "Ae"},
                                                                {QRegularExpression("æ"), "ae"},
                                                                {QRegularExpression(" ?[|/]+ ?"), " // "}};

    // Handle advanced card types
    if (cardName.contains(reSplitCard)) {
        cardName = cardName.split(reSplitCard).join(" // ");
    }

    if (cardName.contains(reDoubleFacedMarker)) {
        QStringList faces = cardName.split(reDoubleFacedMarker);
        cardName = faces.first().trimmed();
    }

    // Remove unnecessary characters
    cardName.remove(reBrace);
    cardName.remove(reRoundBrace); // I'll be entirely honest here, these are split to accommodate just three cards
    cardName.remove(reDigitBrace); // from un-sets that have a word in between round braces at the end
    cardName.remove(reBraceDigit); // very specific format with the set code in () and collectors number after

    // Normalize characters
    for (auto diff = differences.constBegin(); diff != differences.constEnd(); ++diff) {
        cardName.replace(diff.key(), diff.value());
    }

    return cardName;
}