#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include "raw_json_scanner.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>
#include <QVariant>
#include <libcockatrice/card/card_info.h>
#include <utility>

// many users prefer not to see these sets with non english arts
// they will given priority PriorityLowest
const QStringList nonEnglishSets = {"4BB", "FBB", "PS11", "PSAL", "REN", "RIN"};
const QMap<QString, CardSet::Priority> setTypePriorities{
    {"core", CardSet::PriorityPrimary},
    {"expansion", CardSet::PriorityPrimary},

    {"commander", CardSet::PrioritySecondary},
    {"starter", CardSet::PrioritySecondary},
    {"draft_innovation", CardSet::PrioritySecondary},
    {"duel_deck", CardSet::PrioritySecondary},

    {"archenemy", CardSet::PriorityReprint},
    {"arsenal", CardSet::PriorityReprint},
    {"box", CardSet::PriorityReprint},
    {"eternal", CardSet::PriorityReprint},
    {"from_the_vault", CardSet::PriorityReprint},
    {"masterpiece", CardSet::PriorityReprint},
    {"masters", CardSet::PriorityReprint},
    {"memorabilia", CardSet::PriorityReprint},
    {"planechase", CardSet::PriorityReprint},
    {"premium_deck", CardSet::PriorityReprint},
    {"promo", CardSet::PriorityReprint},
    {"spellbook", CardSet::PriorityReprint},
    {"token", CardSet::PriorityReprint},
    {"treasure_chest", CardSet::PriorityReprint},

    {"alchemy", CardSet::PriorityOther},
    {"funny", CardSet::PriorityOther},
    {"minigame", CardSet::PriorityOther},
    {"vanguard", CardSet::PriorityOther},
};

class SetToDownload
{
private:
    QString shortName, longName;
    QDate releaseDate;
    QString setType;
    CardSet::Priority priority;
    // Byte range of this set's object within the importer's raw JSON text. Parsing
    // one set at a time keeps peak memory low instead of holding the whole document.
    RawJson::SetDataRange rawRange;

public:
    const QString &getShortName() const
    {
        return shortName;
    }
    const QString &getLongName() const
    {
        return longName;
    }
    const QString &getSetType() const
    {
        return setType;
    }
    const QDate &getReleaseDate() const
    {
        return releaseDate;
    }
    CardSet::Priority getPriority() const
    {
        return priority;
    }
    const RawJson::SetDataRange &getRawRange() const
    {
        return rawRange;
    }
    SetToDownload(QString _shortName,
                  QString _longName,
                  CardSet::Priority _priority,
                  QString _setType = QString(),
                  const QDate &_releaseDate = QDate())
        : shortName(std::move(_shortName)), longName(std::move(_longName)), releaseDate(_releaseDate),
          setType(std::move(_setType)), priority(_priority)
    {
    }
    void setRawRange(const RawJson::SetDataRange &_rawRange)
    {
        rawRange = _rawRange;
    }
    bool operator<(const SetToDownload &set) const
    {
        return longName.compare(set.longName, Qt::CaseInsensitive) < 0;
    }
};

class SplitCardPart
{
public:
    SplitCardPart(const QString &_name,
                  const QString &_text,
                  const QHash<QString, QString> &_properties,
                  const PrintingInfo &_printingInfo);
    inline const QString &getName() const
    {
        return name;
    }
    inline const QString &getText() const
    {
        return text;
    }
    inline const QHash<QString, QString> &getProperties() const
    {
        return properties;
    }
    inline const PrintingInfo &getPrintingInfo() const
    {
        return printingInfo;
    }

private:
    QString name;
    QString text;
    QHash<QString, QString> properties;
    PrintingInfo printingInfo;
};

class OracleImporter : public QObject
{
    Q_OBJECT
private:
    static const QRegularExpression formatRegex;

    /**
     * The cards, indexed by name.
     */
    CardNameMap cards;

    /**
     * The sets, indexed by short name.
     */
    SetNameMap sets;

    QList<SetToDownload> allSets;

    /**
     * The raw JSON text of the source document, retained for lazy per-set
     * parsing during startImport(). Frees the card data as each set is imported.
     */
    QByteArray rawSetsData;

    CardInfoPtr addCard(QString name,
                        const QString &text,
                        bool isToken,
                        QHash<QString, QString> properties,
                        const QList<CardRelation *> &relatedCards,
                        const PrintingInfo &printingInfo);
signals:
    void setIndexChanged(int cardsImported, int setIndex, const QString &setName);
    void dataReadProgress(int bytesRead, int totalBytes);

public:
    explicit OracleImporter(QObject *parent = nullptr);
    /**
     * Scans the given JSON document for set metadata. Takes the data by value so
     * the wizard can hand over its decompressed buffer without copying it.
     */
    bool readSetsFromByteArray(QByteArray data);
    int startImport();
    bool saveToFile(const QString &fileName, const QString &sourceUrl, const QString &sourceVersion);
    int importCardsFromSet(const CardSetPtr &currentSet, const QJsonArray &cardsList);
    /**
     * @brief Returns the default format rules. The result is memoized on first use and must be treated as immutable.
     */
    const FormatRulesNameMap &createDefaultMagicFormats();
    const CardNameMap &getCardList() const
    {
        return cards;
    }
    QList<SetToDownload> &getSets()
    {
        return allSets;
    }
    const QByteArray &getRawSetsData() const
    {
        return rawSetsData;
    }
    void releaseSetData();
    void clear();
};

#endif
