#ifndef CONTINUUMATRICE_ESSENCE_COUNTER_H
#define CONTINUUMATRICE_ESSENCE_COUNTER_H

#include <QChar>
#include <QString>


namespace EssenceCounter
{
// A single authoritative resource-code order shared by card metadata, the
// client match display, and the dedicated/local multiplayer servers.
// This is a protocol identity list, not a definition of gameplay generation.
inline const QString &symbols()
{
    static const QString value = QStringLiteral("MSOGEAQCJBRLVTPN");
    return value;
}

inline bool isSymbol(QChar code)
{
    return symbols().contains(code);
}

// Names are serialized by Cockatrice's existing ServerInfo_Counter protocol.
// Keep these stable for replays, game snapshots, and reconnects.
inline QString name(QChar code)
{
    return isSymbol(code) ? QStringLiteral("essence:") + code : QString();
}

inline QChar codeFromName(const QString &counterName)
{
    constexpr qsizetype prefixSize = 8; // "essence:"
    if (counterName.size() != prefixSize + 1 ||
        !counterName.startsWith(QStringLiteral("essence:"))) {
        return QChar();
    }
    const QChar code = counterName.at(prefixSize);
    return isSymbol(code) ? code : QChar();
}

// Resource balances cannot be negative. The caller handles overflow-safe
// addition separately; this function only clamps the final amount.
inline int clampBalance(int amount)
{
    return amount < 0 ? 0 : amount;
}

inline bool isEssenceCounter(const QString &counterName)
{
    return !codeFromName(counterName).isNull();
}
} // namespace EssenceCounter

#endif // CONTINUUMATRICE_ESSENCE_COUNTER_H
