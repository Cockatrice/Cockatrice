#ifndef CONTINUUMATRICE_ESSENCE_TYPES_H
#define CONTINUUMATRICE_ESSENCE_TYPES_H

#include <QChar>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>

namespace Essence
{
// Canonical Continuum Universes CCG essence symbols. N is colorless.
// The other codes have no names, colors or game mechanics assigned here.
inline const QString &symbols()
{
    static const QString value = QStringLiteral("MSOGEAQCJBRLVTPN");
    return value;
}

inline bool isSymbol(const QChar symbol)
{
    return symbols().contains(symbol);
}

// Stable ordering for the card's explicitly recorded Essence identity. This
// is a list of distinct codes, NOT a derived resource cost or mana color.
inline QString normalizeIdentity(const QString &input)
{
    const QString uppercase = input.toUpper();
    QString normalized;
    for (const QChar code : symbols()) {
        if (uppercase.contains(code)) {
            normalized.append(code);
        }
    }
    return normalized;
}

// Costs may contain bare or braced symbols, e.g. MSN or {M}{S}{N}.
// Decimal quantities are passed through as generic amounts for display.
// Hybrid/phrexian/other MTG-specific syntax is NOT defined as Essence.
inline QStringList parseCost(const QString &cost)
{
    static const QRegularExpression regex(QStringLiteral(R"(\{([^}]+)\}|(\d+)|([MSOGEAQCJBRLVTPN]))"));
    QStringList result;
    auto matches = regex.globalMatch(cost);
    while (matches.hasNext()) {
        const auto match = matches.next();
        if (!match.captured(1).isEmpty()) {
            const QString token = match.captured(1);
            if (token.size() == 1 && isSymbol(token.at(0))) {
                result.append(token);
            } else {
                // Numeric quantities may also be written in braces: {2}{N}.
                bool isNumber = !token.isEmpty();
                for (const QChar ch : token) {
                    if (!ch.isDigit()) {
                        isNumber = false;
                        break;
                    }
                }
                if (isNumber) {
                    result.append(token);
                }
            }
        } else if (!match.captured(2).isEmpty()) {
            result.append(match.captured(2));
        } else if (!match.captured(3).isEmpty()) {
            result.append(match.captured(3));
        }
    }
    return result;
}
} // namespace Essence

#endif // CONTINUUMATRICE_ESSENCE_TYPES_H
