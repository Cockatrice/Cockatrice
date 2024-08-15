#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <QMap>
#include <QVariant>
#include <game/cards/card_database.h>
#include <utility>

// many users prefer not to see these sets with non english arts
const QStringList nonEnglishSets = {"4BB", "FBB", "PS11", "PSAL", "REN", "RIN"};
const QMap<QString, CardSet::Priority> setTypePriorities{
    {"core",             CardSet::Priority::PRIMARY},
    {"expansion",        CardSet::Priority::PRIMARY},

    {"commander",        CardSet::Priority::SECONDARY},
    {"starter",          CardSet::Priority::SECONDARY},

    {"archenemy",        CardSet::Priority::REPRINT},
    {"arsenal",          CardSet::Priority::REPRINT},
    {"box",              CardSet::Priority::REPRINT},
    {"draft_innovation", CardSet::Priority::REPRINT},
    {"duel_deck",        CardSet::Priority::REPRINT},
    {"from_the_vault",   CardSet::Priority::REPRINT},
    {"masterpiece",      CardSet::Priority::REPRINT},
    {"masters",          CardSet::Priority::REPRINT},
    {"memorabilia",      CardSet::Priority::REPRINT},
    {"planechase",       CardSet::Priority::REPRINT},
    {"premium_deck",     CardSet::Priority::REPRINT},
    {"promo",            CardSet::Priority::REPRINT},
    {"spellbook",        CardSet::Priority::REPRINT},
    {"token",            CardSet::Priority::REPRINT},
    {"treasure_chest",   CardSet::Priority::REPRINT},

    {"alchemy",          CardSet::Priority::OTHER},
    {"funny",            CardSet::Priority::OTHER},
    {"minigame",         CardSet::Priority::OTHER},
    {"vanguard",         CardSet::Priority::OTHER},
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
    SplitCardPart(const QString &_name, const QString &_text, const QVariantHash &_properties, CardInfoPerSet setInfo);
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
    inline const CardInfoPerSet &getSetInfo() const
    {
        return setInfo;
    }

private:
    QString name;
    QString text;
    QVariantHash properties;
    CardInfoPerSet setInfo;
};

class OracleImporter : public CardDatabase
{
    Q_OBJECT
private:
    const QStringList mainCardTypes = {"Planeswalker", "Creature", "Land",       "Sorcery",
                                       "Instant",      "Artifact", "Enchantment"};
    QList<SetToDownload> allSets;
    QVariantMap setsMap;
    QString dataDir;

    QString getMainCardType(const QStringList &typeList);
    CardInfoPtr addCard(QString name,
                        QString text,
                        bool isToken,
                        QVariantHash properties,
                        QList<CardRelation *> &relatedCards,
                        CardInfoPerSet setInfo);
signals:
    void setIndexChanged(int cardsImported, int setIndex, const QString &setName);
    void dataReadProgress(int bytesRead, int totalBytes);

public:
    explicit OracleImporter(const QString &_dataDir, QObject *parent = nullptr);
    CardSet::Priority getSetPriority(QString &setType, QString &shortName);
    bool readSetsFromByteArray(const QByteArray &data);
    int startImport();
    bool saveToFile(const QString &fileName, const QString &sourceUrl, const QString &sourceVersion);
    int importCardsFromSet(const CardSetPtr &currentSet, const QList<QVariant> &cards, bool skipSpecialNums = true);
    QList<SetToDownload> &getSets()
    {
        return allSets;
    }
    const QString &getDataDir() const
    {
        return dataDir;
    }
    void clear();

protected:
    inline QString getStringPropertyFromMap(const QVariantMap &card, const QString &propertyName);
    void sortAndReduceColors(QString &colors);
};

#endif
