#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

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
    QList<QVariant> cards;
    QDate releaseDate;
    QString setType;
    CardSet::Priority priority;

public:
    const QString &getShortName() const
    {
        return shortName;
    }
    const QString &getLongName() const
    {
        return longName;
    }
    const QList<QVariant> &getCards() const
    {
        return cards;
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
    SetToDownload(QString _shortName,
                  QString _longName,
                  QList<QVariant> _cards,
                  CardSet::Priority _priority,
                  QString _setType = QString(),
                  const QDate &_releaseDate = QDate())
        : shortName(std::move(_shortName)), longName(std::move(_longName)), cards(std::move(_cards)),
          releaseDate(_releaseDate), setType(std::move(_setType)), priority(_priority)
    {
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
                  const QVariantHash &_properties,
                  const PrintingInfo &_printingInfo);
    inline const QString &getName() const
    {
        return name;
    }
    inline const QString &getText() const
    {
        return text;
    }
    inline const QVariantHash &getProperties() const
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
    QVariantHash properties;
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

    CardInfoPtr addCard(QString name,
                        const QString &text,
                        bool isToken,
                        QVariantHash properties,
                        const QList<CardRelation *> &relatedCards,
                        const PrintingInfo &printingInfo);
signals:
    void setIndexChanged(int cardsImported, int setIndex, const QString &setName);
    void dataReadProgress(int bytesRead, int totalBytes);

public:
    explicit OracleImporter(QObject *parent = nullptr);
    bool readSetsFromByteArray(const QByteArray &data);
    int startImport();
    bool saveToFile(const QString &fileName, const QString &sourceUrl, const QString &sourceVersion);
    int importCardsFromSet(const CardSetPtr &currentSet, const QList<QVariant> &cardsList);
    const CardNameMap &getCardList() const
    {
        return cards;
    }
    QList<SetToDownload> &getSets()
    {
        return allSets;
    }
    void clear();
};

#endif
