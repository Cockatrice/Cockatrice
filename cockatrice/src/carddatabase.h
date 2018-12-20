#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QBasicMutex>
#include <QDate>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

class CardDatabase;
class CardInfo;
class CardSet;
class CardRelation;
class ICardDatabaseParser;

typedef QMap<QString, QString> QStringMap;
typedef QMap<QString, int> MuidMap;
typedef QSharedPointer<CardInfo> CardInfoPtr;
typedef QSharedPointer<CardSet> CardSetPtr;

Q_DECLARE_METATYPE(CardInfoPtr)

class CardSet : public QList<CardInfoPtr>
{
private:
    QString shortName, longName;
    unsigned int sortKey;
    QDate releaseDate;
    QString setType;
    bool enabled, isknown;

public:
    explicit CardSet(const QString &_shortName = QString(),
                     const QString &_longName = QString(),
                     const QString &_setType = QString(),
                     const QDate &_releaseDate = QDate());
    static CardSetPtr newInstance(const QString &_shortName = QString(),
                                  const QString &_longName = QString(),
                                  const QString &_setType = QString(),
                                  const QDate &_releaseDate = QDate());
    QString getCorrectedShortName() const;
    QString getShortName() const
    {
        return shortName;
    }
    QString getLongName() const
    {
        return longName;
    }
    QString getSetType() const
    {
        return setType;
    }
    QDate getReleaseDate() const
    {
        return releaseDate;
    }
    void setLongName(const QString &_longName)
    {
        longName = _longName;
    }
    void setSetType(const QString &_setType)
    {
        setType = _setType;
    }
    void setReleaseDate(const QDate &_releaseDate)
    {
        releaseDate = _releaseDate;
    }

    void loadSetOptions();
    int getSortKey() const
    {
        return sortKey;
    }
    void setSortKey(unsigned int _sortKey);
    bool getEnabled() const
    {
        return enabled;
    }
    void setEnabled(bool _enabled);
    bool getIsKnown() const
    {
        return isknown;
    }
    void setIsKnown(bool _isknown);

    // Determine incomplete sets.
    bool getIsKnownIgnored() const
    {
        return longName.length() + setType.length() + releaseDate.toString().length() == 0;
    }
};

class SetList : public QList<CardSetPtr>
{
private:
    class KeyCompareFunctor;

public:
    void sortByKey();
    void guessSortKeys();
    void enableAllUnknown();
    void enableAll();
    void markAllAsKnown();
    int getEnabledSetsNum();
    int getUnknownSetsNum();
    QStringList getUnknownSetsNames();
};

class CardInfo : public QObject
{
    Q_OBJECT
private:
    CardInfoPtr smartThis;
    QString name;

    /*
     * The name without punctuation or capitalization, for better card tag name
     * recognition.
     */
    QString simpleName;

    bool isToken;
    SetList sets;
    QString manacost;
    QString cmc;
    QString cardtype;
    QString powtough;
    QString text;
    QStringList colors;

    // the cards i'm related to
    QList<CardRelation *> relatedCards;

    // the card i'm reverse-related to
    QList<CardRelation *> reverseRelatedCards;

    // the cards thare are reverse-related to me
    QList<CardRelation *> reverseRelatedCardsToMe;

    QString setsNames;

    bool upsideDownArt;
    QString loyalty;
    QStringMap customPicURLs;
    MuidMap muIds;
    QStringMap uuIds;
    QStringMap collectorNumbers;
    QStringMap rarities;
    bool cipt;
    int tableRow;
    QString pixmapCacheKey;

public:
    explicit CardInfo(const QString &_name = QString(),
                      bool _isToken = false,
                      const QString &_manacost = QString(),
                      const QString &_cmc = QString(),
                      const QString &_cardtype = QString(),
                      const QString &_powtough = QString(),
                      const QString &_text = QString(),
                      const QStringList &_colors = QStringList(),
                      const QList<CardRelation *> &_relatedCards = QList<CardRelation *>(),
                      const QList<CardRelation *> &_reverseRelatedCards = QList<CardRelation *>(),
                      bool _upsideDownArt = false,
                      const QString &_loyalty = QString(),
                      bool _cipt = false,
                      int _tableRow = 0,
                      const SetList &_sets = SetList(),
                      const QStringMap &_customPicURLs = QStringMap(),
                      MuidMap _muids = MuidMap(),
                      QStringMap _uuIds = QStringMap(),
                      QStringMap _collectorNumbers = QStringMap(),
                      QStringMap _rarities = QStringMap());
    ~CardInfo() override;

    static CardInfoPtr newInstance(const QString &_name = QString(),
                                   bool _isToken = false,
                                   const QString &_manacost = QString(),
                                   const QString &_cmc = QString(),
                                   const QString &_cardtype = QString(),
                                   const QString &_powtough = QString(),
                                   const QString &_text = QString(),
                                   const QStringList &_colors = QStringList(),
                                   const QList<CardRelation *> &_relatedCards = QList<CardRelation *>(),
                                   const QList<CardRelation *> &_reverseRelatedCards = QList<CardRelation *>(),
                                   bool _upsideDownArt = false,
                                   const QString &_loyalty = QString(),
                                   bool _cipt = false,
                                   int _tableRow = 0,
                                   const SetList &_sets = SetList(),
                                   const QStringMap &_customPicURLs = QStringMap(),
                                   MuidMap _muids = MuidMap(),
                                   QStringMap _uuIds = QStringMap(),
                                   QStringMap _collectorNumbers = QStringMap(),
                                   QStringMap _rarities = QStringMap());

    void setSmartPointer(CardInfoPtr _ptr)
    {
        smartThis = _ptr;
    }

    inline const QString &getName() const
    {
        return name;
    }
    inline const QString &getSetsNames() const
    {
        return setsNames;
    }
    const QString &getSimpleName() const
    {
        return simpleName;
    }
    bool getIsToken() const
    {
        return isToken;
    }
    const SetList &getSets() const
    {
        return sets;
    }
    inline const QString &getManaCost() const
    {
        return manacost;
    }
    inline const QString &getCmc() const
    {
        return cmc;
    }
    inline const QString &getCardType() const
    {
        return cardtype;
    }
    inline const QString &getPowTough() const
    {
        return powtough;
    }
    const QString &getText() const
    {
        return text;
    }
    const QString &getPixmapCacheKey() const
    {
        return pixmapCacheKey;
    }
    const QString &getLoyalty() const
    {
        return loyalty;
    }
    bool getCipt() const
    {
        return cipt;
    }
    // void setManaCost(const QString &_manaCost) { manacost = _manaCost; emit cardInfoChanged(smartThis); }
    // void setCmc(const QString &_cmc) { cmc = _cmc; emit cardInfoChanged(smartThis); }
    void setCardType(const QString &_cardType)
    {
        cardtype = _cardType;
        emit cardInfoChanged(smartThis);
    }
    void setPowTough(const QString &_powTough)
    {
        powtough = _powTough;
        emit cardInfoChanged(smartThis);
    }
    void setText(const QString &_text)
    {
        text = _text;
        emit cardInfoChanged(smartThis);
    }
    void setColors(const QStringList &_colors)
    {
        colors = _colors;
        emit cardInfoChanged(smartThis);
    }
    const QChar getColorChar() const;
    const QStringList &getColors() const
    {
        return colors;
    }
    const QList<CardRelation *> &getRelatedCards() const
    {
        return relatedCards;
    }
    const QList<CardRelation *> &getReverseRelatedCards() const
    {
        return reverseRelatedCards;
    }
    const QList<CardRelation *> &getReverseRelatedCards2Me() const
    {
        return reverseRelatedCardsToMe;
    }
    void resetReverseRelatedCards2Me();
    void addReverseRelatedCards2Me(CardRelation *cardRelation)
    {
        reverseRelatedCardsToMe.append(cardRelation);
    }
    bool getUpsideDownArt() const
    {
        return upsideDownArt;
    }
    QString getCustomPicURL(const QString &set) const
    {
        return customPicURLs.value(set);
    }
    int getMuId(const QString &set) const
    {
        return muIds.value(set);
    }
    QString getUuId(const QString &set) const
    {
        return uuIds.value(set);
    }
    QString getCollectorNumber(const QString &set) const
    {
        return collectorNumbers.value(set);
    }
    QString getRarity(const QString &set) const
    {
        return rarities.value(set);
    }
    QStringMap getRarities() const
    {
        return rarities;
    }
    QString getMainCardType() const;
    QString getCorrectedName() const;
    int getTableRow() const
    {
        return tableRow;
    }
    void setTableRow(int _tableRow)
    {
        tableRow = _tableRow;
    }
    // void setLoyalty(int _loyalty) { loyalty = _loyalty; emit cardInfoChanged(smartThis); }
    // void setCustomPicURL(const QString &_set, const QString &_customPicURL) { customPicURLs.insert(_set,
    // _customPicURL); }
    void setSet(const CardSetPtr &_set)
    {
        sets.append(_set);
        refreshCachedSetNames();
    }
    void setMuId(const QString &_set, const int &_muId)
    {
        muIds.insert(_set, _muId);
    }
    void setUuId(const QString &_set, const QString &_uuId)
    {
        uuIds.insert(_set, _uuId);
    }
    void setSetNumber(const QString &_set, const QString &_setNumber)
    {
        collectorNumbers.insert(_set, _setNumber);
    }
    void setRarity(const QString &_set, const QString &_setNumber)
    {
        rarities.insert(_set, _setNumber);
    }
    void addToSet(CardSetPtr set);
    void emitPixmapUpdated()
    {
        emit pixmapUpdated();
    }
    void refreshCachedSetNames();

    /**
     * Simplify a name to have no punctuation and lowercase all letters, for
     * less strict name-matching.
     */
    static QString simplifyName(const QString &name);

signals:
    void pixmapUpdated();
    void cardInfoChanged(CardInfoPtr card);
};

enum LoadStatus
{
    Ok,
    VersionTooOld,
    Invalid,
    NotLoaded,
    FileError,
    NoCards
};

typedef QHash<QString, CardInfoPtr> CardNameMap;
typedef QHash<QString, CardSetPtr> SetNameMap;

class CardDatabase : public QObject
{
    Q_OBJECT
protected:
    /*
     * The cards, indexed by name.
     */
    CardNameMap cards;

    /**
     * The cards, indexed by their simple name.
     */
    CardNameMap simpleNameCards;

    /*
     * The sets, indexed by short name.
     */
    SetNameMap sets;

    LoadStatus loadStatus;

    QVector<ICardDatabaseParser *> availableParsers;

private:
    CardInfoPtr getCardFromMap(const CardNameMap &cardMap, const QString &cardName) const;
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();

    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(), *clearDatabaseMutex = new QBasicMutex(),
                *loadFromFileMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    static const char *TOKENS_SETNAME;

    explicit CardDatabase(QObject *parent = nullptr);
    ~CardDatabase() override;
    void clear();
    void removeCard(CardInfoPtr card);
    CardInfoPtr getCard(const QString &cardName) const;
    QList<CardInfoPtr> getCards(const QStringList &cardNames) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    CardSetPtr getSet(const QString &setName);
    QList<CardInfoPtr> getCardList() const
    {
        return cards.values();
    }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName);
    bool saveCustomTokensToFile();
    QStringList getAllColors() const;
    QStringList getAllMainCardTypes() const;
    LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }
    void enableAllUnknownSets();
    void markAllSetsAsKnown();
    void notifyEnabledSetsChanged();

public slots:
    LoadStatus loadCardDatabases();
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
private slots:
    LoadStatus loadCardDatabase(const QString &path);
signals:
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();
    void cardDatabaseEnabledSetsChanged();
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);
};

class CardRelation : public QObject
{
    Q_OBJECT
private:
    QString name;
    bool doesAttach;
    bool isCreateAllExclusion;
    bool isVariableCount;
    int defaultCount;

public:
    explicit CardRelation(const QString &_name = QString(),
                          bool _doesAttach = false,
                          bool _isCreateAllExclusion = false,
                          bool _isVariableCount = false,
                          int _defaultCount = 1);

    inline const QString &getName() const
    {
        return name;
    }
    bool getDoesAttach() const
    {
        return doesAttach;
    }
    bool getCanCreateAnother() const
    {
        return !doesAttach;
    }
    bool getIsCreateAllExclusion() const
    {
        return isCreateAllExclusion;
    }
    bool getIsVariable() const
    {
        return isVariableCount;
    }
    int getDefaultCount() const
    {
        return defaultCount;
    }
};
#endif