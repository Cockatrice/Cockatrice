#include "card_database_cache.h"

#include "../card_info.h"
#include "../format/format_legality_rules.h"
#include "../printing/printing_info.h"
#include "../relation/card_relation.h"
#include "../relation/card_relation_type.h"
#include "../set/card_set.h"
#include "card_database_loader.h"

#include <QBuffer>
#include <QDataStream>
#include <QElapsedTimer>
#include <QFile>
#include <QSaveFile>

namespace
{
constexpr quint32 CACHE_MAGIC = 0x43445243; // "CDRC"
constexpr quint32 CACHE_VERSION = 2;

// ---- Primitives -----------------------------------------------------------

void writeString(QDataStream &out, const QString &s)
{
    out << s;
}

QString readString(QDataStream &in)
{
    QString s;
    in >> s;
    return s;
}

// Stores a QHash<QString, QString> as a single pre-serialized blob. The reader keeps the
// blob as-is and materializes the QHash<QString, QString> lazily on first query, which is
// what removes the allocation storm from database load (see card_info.cpp /
// printing_info.cpp).
void writeHashBlob(QDataStream &out, const QHash<QString, QString> &h)
{
    QByteArray blob;
    QDataStream blobOut(&blob, QIODevice::WriteOnly);
    blobOut.setVersion(QDataStream::Qt_6_4);
    blobOut << h;
    out << static_cast<quint32>(blob.size());
    out.writeRawData(blob.constData(), blob.size());
}

QByteArray readHashBlob(QDataStream &in)
{
    quint32 len = 0;
    in >> len;
    if (in.status() != QDataStream::Ok || static_cast<qint64>(len) > in.device()->bytesAvailable()) {
        return {};
    }
    QByteArray blob(len, Qt::Uninitialized);
    in.readRawData(blob.data(), static_cast<int>(len));
    return blob;
}

void writeDate(QDataStream &out, const QDate &d)
{
    out << d;
}

QDate readDate(QDataStream &in)
{
    QDate d;
    in >> d;
    return d;
}

// ---- CardRelation ----------------------------------------------------------

void writeRelation(QDataStream &out, const CardRelation *rel)
{
    writeString(out, rel->getName());
    out << static_cast<quint32>(rel->getAttachType());
    out << rel->getIsCreateAllExclusion();
    out << rel->getIsVariable();
    out << rel->getDefaultCount();
    out << rel->getIsPersistent();
    out << rel->getIsFaceDown();
}

CardRelation *readRelation(QDataStream &in)
{
    QString name = readString(in);
    quint32 attachType = 0;
    bool isExclusion = false;
    bool isVariable = false;
    int defaultCount = 1;
    bool isPersistent = false;
    bool isFaceDown = false;
    in >> attachType;
    in >> isExclusion;
    in >> isVariable;
    in >> defaultCount;
    in >> isPersistent;
    in >> isFaceDown;
    return new CardRelation(name, static_cast<CardRelationType>(attachType), isExclusion, isVariable, defaultCount,
                            isPersistent, isFaceDown);
}

// ---- PrintingInfo ----------------------------------------------------------

// A printing references its set by short name; the CardSetPtr is resolved after
// all sets have been reconstructed on read.
void writePrinting(QDataStream &out, const PrintingInfo &p)
{
    writeString(out, p.getSet() ? p.getSet()->getShortName() : QString());
    writeHashBlob(out, p.getPropertiesHash());
}

PrintingInfo readPrinting(QDataStream &in, const SetNameMap &sets)
{
    QString setName = readString(in);
    QByteArray propsBlob = readHashBlob(in);
    auto set = sets.value(setName);

    return PrintingInfo(set, LazyPropertiesHash(propsBlob));
}

// ---- CardSet ---------------------------------------------------------------

void writeSet(QDataStream &out, const CardSetPtr &set)
{
    writeString(out, set->getShortName());
    writeString(out, set->getLongName());
    writeString(out, set->getSetType());
    writeDate(out, set->getReleaseDate());
    out << static_cast<quint32>(set->getPriority());
}

CardSetPtr readSet(QDataStream &in, ICardSetPriorityController *priorityController)
{
    QString shortName = readString(in);
    QString longName = readString(in);
    QString setType = readString(in);
    QDate releaseDate = readDate(in);
    quint32 priority = 0;
    in >> priority;
    return CardSet::newInstance(priorityController, shortName, longName, setType, releaseDate,
                                static_cast<CardSet::Priority>(priority));
}

// ---- CardInfo --------------------------------------------------------------

void writeCard(QDataStream &out, const CardInfoPtr &card)
{
    writeString(out, card->getName());
    writeString(out, card->getText());
    out << card->getIsToken();
    writeHashBlob(out, card->getPropertiesHash());

    CardInfo::UiAttributes ui = card->getUiAttributes();
    out << ui.cipt;
    out << ui.landscapeOrientation;
    out << ui.tableRow;
    out << ui.upsideDownArt;

    // Precomputed derived state, so the reader can skip simplifyName() and the
    // per-printing alt-name scan entirely.
    writeString(out, card->getSimpleName());
    {
        const QSet<QString> altNames = card->getAltNames();
        out << static_cast<quint32>(altNames.size());
        for (const QString &alt : altNames) {
            writeString(out, alt);
        }
    }

    // setsToPrintings
    const SetToPrintingsMap sets = card->getSets();
    out << static_cast<quint32>(sets.size());
    for (auto it = sets.constBegin(); it != sets.constEnd(); ++it) {
        writeString(out, it.key());
        out << static_cast<quint32>(it.value().size());
        for (const PrintingInfo &p : it.value()) {
            writePrinting(out, p);
        }
    }

    // related cards
    const QList<CardRelation *> related = card->getRelatedCards();
    out << static_cast<quint32>(related.size());
    for (const CardRelation *rel : related) {
        writeRelation(out, rel);
    }

    // reverse-related cards (the reverseRelatedCards list, not the computed 2Me)
    const QList<CardRelation *> reverse = card->getReverseRelatedCards();
    out << static_cast<quint32>(reverse.size());
    for (const CardRelation *rel : reverse) {
        writeRelation(out, rel);
    }
}

CardInfoPtr readCard(QDataStream &in, const SetNameMap &sets)
{
    QString name = readString(in);
    QString text = readString(in);
    bool isToken = false;
    in >> isToken;
    QByteArray propertiesBlob = readHashBlob(in);

    CardInfo::UiAttributes ui;
    in >> ui.cipt;
    in >> ui.landscapeOrientation;
    in >> ui.tableRow;
    in >> ui.upsideDownArt;

    // Precomputed derived state, restored directly to skip simplifyName() and the
    // per-printing alt-name scan.
    QString simpleName = readString(in);
    QSet<QString> altNames;
    quint32 altNameCount = 0;
    in >> altNameCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    altNames.reserve(altNameCount);
    for (quint32 i = 0; i < altNameCount; ++i) {
        altNames.insert(readString(in));
    }

    SetToPrintingsMap cardSets;
    quint32 setCount = 0;
    in >> setCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    for (quint32 i = 0; i < setCount; ++i) {
        QString setName = readString(in);
        quint32 printingCount = 0;
        in >> printingCount;
        if (in.status() != QDataStream::Ok) {
            return nullptr;
        }
        QList<PrintingInfo> printings;
        printings.reserve(printingCount);
        for (quint32 j = 0; j < printingCount; ++j) {
            printings.append(readPrinting(in, sets));
        }
        cardSets.insert(setName, printings);
    }

    QList<CardRelation *> related;
    quint32 relatedCount = 0;
    in >> relatedCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    related.reserve(relatedCount);
    for (quint32 i = 0; i < relatedCount; ++i) {
        related.append(readRelation(in));
    }

    QList<CardRelation *> reverse;
    quint32 reverseCount = 0;
    in >> reverseCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    reverse.reserve(reverseCount);
    for (quint32 i = 0; i < reverseCount; ++i) {
        reverse.append(readRelation(in));
    }

    return CardInfo::newInstance(name, text, isToken, propertiesBlob, related, reverse, cardSets, ui, simpleName,
                                 altNames, false);
}

// ---- FormatRules -----------------------------------------------------------

void writeFormat(QDataStream &out, const FormatRulesPtr &format)
{
    writeString(out, format->formatName);
    out << format->minDeckSize;
    out << format->maxDeckSize;
    out << format->maxSideboardSize;

    out << static_cast<quint32>(format->allowedCounts.size());
    for (const AllowedCount &ac : format->allowedCounts) {
        out << ac.max;
        writeString(out, ac.label);
    }

    out << static_cast<quint32>(format->exceptions.size());
    for (const ExceptionRule &ex : format->exceptions) {
        out << static_cast<quint32>(ex.conditions.size());
        for (const CardCondition &cond : ex.conditions) {
            writeString(out, cond.field);
            writeString(out, cond.matchType);
            writeString(out, cond.value);
        }
        out << ex.maxCopies;
    }
}

FormatRulesPtr readFormat(QDataStream &in)
{
    FormatRulesPtr format(new FormatRules());
    format->formatName = readString(in);
    in >> format->minDeckSize;
    in >> format->maxDeckSize;
    in >> format->maxSideboardSize;

    quint32 allowedCount = 0;
    in >> allowedCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    for (quint32 i = 0; i < allowedCount; ++i) {
        AllowedCount ac;
        in >> ac.max;
        ac.label = readString(in);
        format->allowedCounts.append(ac);
    }

    quint32 exceptionCount = 0;
    in >> exceptionCount;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    for (quint32 i = 0; i < exceptionCount; ++i) {
        ExceptionRule ex;
        quint32 condCount = 0;
        in >> condCount;
        if (in.status() != QDataStream::Ok) {
            return nullptr;
        }
        for (quint32 j = 0; j < condCount; ++j) {
            CardCondition cond;
            cond.field = readString(in);
            cond.matchType = readString(in);
            cond.value = readString(in);
            ex.conditions.append(cond);
        }
        in >> ex.maxCopies;
        format->exceptions.append(ex);
    }
    return format;
}

} // namespace

bool CardDatabaseCache::write(const QString &cachePath, const CardDatabaseData &data, const QByteArray &sourceHash)
{
    QSaveFile file(cachePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_4);

    // Header
    out << CACHE_MAGIC;
    out << CACHE_VERSION;
    out << sourceHash;

    // Sets
    out << static_cast<quint32>(data.sets.size());
    for (const CardSetPtr &set : data.sets) {
        writeSet(out, set);
    }

    // Cards
    const quint32 cardCount = static_cast<quint32>(data.cards.size());
    out << cardCount;
    for (const CardInfoPtr &card : data.cards) {
        writeCard(out, card);
    }

    // Formats
    out << static_cast<quint32>(data.formats.size());
    for (const FormatRulesPtr &format : data.formats) {
        writeFormat(out, format);
    }

    return file.commit();
}

bool CardDatabaseCache::read(const QString &cachePath,
                             CardDatabaseData &data,
                             const QByteArray &sourceHash,
                             ICardSetPriorityController *priorityController)
{
    QFile file(cachePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Read the whole cache into memory up front; deserialization then works on an
    // in-memory buffer with no further disk I/O.
    QByteArray raw = file.readAll();
    if (raw.isEmpty()) {
        return false;
    }

    QBuffer buffer(&raw);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    in.setVersion(QDataStream::Qt_6_4);

    quint32 magic = 0;
    quint32 version = 0;
    QByteArray storedHash;
    in >> magic >> version >> storedHash;
    if (magic != CACHE_MAGIC || version != CACHE_VERSION || storedHash != sourceHash) {
        return false;
    }

    QElapsedTimer deserializeTimer;
    deserializeTimer.start();

    // Sets
    quint32 setCount = 0;
    in >> setCount;
    if (in.status() != QDataStream::Ok) {
        return false;
    }
    for (quint32 i = 0; i < setCount; ++i) {
        CardSetPtr set = readSet(in, priorityController);
        data.sets.insert(set->getShortName(), set);
    }

    // Cards
    quint32 cardCount = 0;
    in >> cardCount;
    if (in.status() != QDataStream::Ok) {
        return false;
    }
    if (cardCount > 0) {
        data.cards.reserve(static_cast<int>(cardCount));
        for (quint32 i = 0; i < cardCount; ++i) {
            CardInfoPtr card = readCard(in, data.sets);
            if (card == nullptr) {
                return false;
            }
            data.cards.insert(card->getName(), card);
            data.simpleNameCards.insert(card->getSimpleName(), card);
        }

        for (const CardInfoPtr &card : data.cards) {
            for (const auto &printings : card->getSets()) {
                for (const PrintingInfo &printing : printings) {
                    if (auto set = printing.getSet()) {
                        set->append(card);
                        break;
                    }
                }
            }
        }
    }

    // Formats
    quint32 formatCount = 0;
    in >> formatCount;
    if (in.status() != QDataStream::Ok) {
        return false;
    }
    for (quint32 i = 0; i < formatCount; ++i) {
        FormatRulesPtr format = readFormat(in);
        if (format == nullptr) {
            return false;
        }
        data.formats.insert(format->formatName.toLower(), format);
    }

    qCInfo(CardDatabaseLoadingLog) << "[cache] read + deserialize" << deserializeTimer.elapsed() << "ms for"
                                   << cardCount << "cards";

    if (in.status() != QDataStream::Ok) {
        return false;
    }

    return file.error() == QFile::NoError;
}
