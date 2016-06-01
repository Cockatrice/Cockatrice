#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QHash>
#include <QPixmap>
#include <QMap>
#include <QDate>
#include <QDataStream>
#include <QList>
#include <QXmlStreamReader>

class CardDatabase;
class CardInfo;

typedef QMap<QString, QString> QStringMap;

// If we don't typedef this, CardInfo::CardInfo will refuse to compile on OS X < 10.9
typedef QMap<QString, int> MuidMap;

class CardSet : public QList<CardInfo *> {
private:
    QString shortName, longName;
    unsigned int sortKey;
    QDate releaseDate;
    QString setType;
    bool enabled, isknown;
public:
    CardSet(const QString &_shortName = QString(), const QString &_longName = QString(), const QString &_setType = QString(), const QDate &_releaseDate = QDate());
    QString getCorrectedShortName() const;
    QString getShortName() const { return shortName; }
    QString getLongName() const { return longName; }
    QString getSetType() const { return setType; }
    QDate getReleaseDate() const { return releaseDate; }
    void setLongName(QString & _longName) { longName = _longName; }
    void setSetType(QString & _setType) { setType = _setType; }
    void setReleaseDate(QDate & _releaseDate) { releaseDate = _releaseDate; }

    void loadSetOptions();
    int getSortKey() const { return sortKey; }
    void setSortKey(unsigned int _sortKey);
    bool getEnabled() const { return enabled; }
    void setEnabled(bool _enabled);
    bool getIsKnown() const { return isknown; }
    void setIsKnown(bool _isknown);
};

class SetList : public QList<CardSet *> {
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

class CardInfo : public QObject {
    Q_OBJECT
private:
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
    QStringList relatedCards;
    // the card i'm reverse-related to
    QStringList reverseRelatedCards;
    // the cards thare are reverse-related to me
    QStringList reverseRelatedCardsToMe;
    QString setsNames;
    bool upsideDownArt;
    int loyalty;
    QStringMap customPicURLs;
    MuidMap muIds;
    QStringMap setNumbers;
    QStringMap rarities;
    bool cipt;
    int tableRow;
    QString pixmapCacheKey;
public:
    CardInfo(const QString &_name = QString(),
        bool _isToken = false,
        const QString &_manacost = QString(),
        const QString &_cmc = QString(),
        const QString &_cardtype = QString(),
        const QString &_powtough = QString(),
        const QString &_text = QString(),
        const QStringList &_colors = QStringList(),
        const QStringList &_relatedCards = QStringList(),
        const QStringList &_reverseRelatedCards = QStringList(),
        bool _upsideDownArt = false,
        int _loyalty = 0,
        bool _cipt = false,
        int _tableRow = 0,
        const SetList &_sets = SetList(),
        const QStringMap &_customPicURLs = QStringMap(),
        MuidMap muids = MuidMap(),
        QStringMap _setNumbers = QStringMap(),
        QStringMap _rarities = QStringMap()
        );
    ~CardInfo();
    inline const QString &getName() const { return name; }
    inline const QString &getSetsNames() const { return setsNames; }
    const QString &getSimpleName() const { return simpleName; }
    bool getIsToken() const { return isToken; }
    const SetList &getSets() const { return sets; }
    inline const QString &getManaCost() const { return manacost; }
    inline const QString &getCmc() const { return cmc; }
    inline const QString &getCardType() const { return cardtype; }
    inline const QString &getPowTough() const { return powtough; }
    const QString &getText() const { return text; }
    const QString &getPixmapCacheKey() const { return pixmapCacheKey; }
    const int &getLoyalty() const { return loyalty; }
    bool getCipt() const { return cipt; }
    void setManaCost(const QString &_manaCost) { manacost = _manaCost; emit cardInfoChanged(this); }
    void setCmc(const QString &_cmc) { cmc = _cmc; emit cardInfoChanged(this); }
    void setCardType(const QString &_cardType) { cardtype = _cardType; emit cardInfoChanged(this); }
    void setPowTough(const QString &_powTough) { powtough = _powTough; emit cardInfoChanged(this); }
    void setText(const QString &_text) { text = _text; emit cardInfoChanged(this); }
    void setColors(const QStringList &_colors) { colors = _colors; emit cardInfoChanged(this); }
    const QChar getColorChar() const;
    const QStringList &getColors() const { return colors; }
    const QStringList &getRelatedCards() const { return relatedCards; }
    const QStringList &getReverseRelatedCards() const { return reverseRelatedCards; }
    const QStringList &getReverseRelatedCards2Me() const { return reverseRelatedCardsToMe; }
    void resetReverseRelatedCards2Me() { reverseRelatedCardsToMe = QStringList(); }
    void addReverseRelatedCards2Me(QString & cardName) { reverseRelatedCardsToMe.append(cardName); }
    bool getUpsideDownArt() const { return upsideDownArt; }
    QString getCustomPicURL(const QString &set) const { return customPicURLs.value(set); }
    int getMuId(const QString &set) const { return muIds.value(set); }
    QString getSetNumber(const QString &set) const { return setNumbers.value(set); }
    QString getRarity(const QString &set) const { return rarities.value(set); }
    QStringMap getRarities() const { return rarities; }
    QString getMainCardType() const;
    QString getCorrectedName() const;
    int getTableRow() const { return tableRow; }
    void setTableRow(int _tableRow) { tableRow = _tableRow; }
    void setLoyalty(int _loyalty) { loyalty = _loyalty; emit cardInfoChanged(this); }
    void setCustomPicURL(const QString &_set, const QString &_customPicURL) { customPicURLs.insert(_set, _customPicURL); }
    void setMuId(const QString &_set, const int &_muId) { muIds.insert(_set, _muId); }
    void setSetNumber(const QString &_set, const QString &_setNumber) { setNumbers.insert(_set, _setNumber); }
    void setRarity(const QString &_set, const QString &_setNumber) { rarities.insert(_set, _setNumber); }
    void addToSet(CardSet *set);
    void emitPixmapUpdated() { emit pixmapUpdated(); }
    void refreshCachedSetNames();

    /**
     * Simplify a name to have no punctuation and lowercase all letters, for
     * less strict name-matching.
     */
    static QString simplifyName(const QString &name);
signals:
    void pixmapUpdated();
    void cardInfoChanged(CardInfo *card);
};

enum LoadStatus { Ok, VersionTooOld, Invalid, NotLoaded, FileError, NoCards };

typedef QHash<QString, CardInfo *> CardNameMap;
typedef QHash<QString, CardSet *> SetNameMap;

class CardDatabase : public QObject {
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
private:
    static const int versionNeeded;
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);

    CardInfo *getCardFromMap(const CardNameMap &cardMap, const QString &cardName) const;
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();
public:
    static const char* TOKENS_SETNAME;

    CardDatabase(QObject *parent = 0);
    ~CardDatabase();
    void clear();
    void addCard(CardInfo *card);
    void removeCard(CardInfo *card);
    CardInfo *getCard(const QString &cardName) const;
    QList <CardInfo *> getCards(const QStringList &cardNames) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    CardInfo *getCardBySimpleName(const QString &cardName) const;

    CardSet *getSet(const QString &setName);
    QList<CardInfo *> getCardList() const { return cards.values(); }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName);
    bool saveToFile(const QString &fileName, bool tokens = false);
    bool saveCustomTokensToFile();
    QStringList getAllColors() const;
    QStringList getAllMainCardTypes() const;
    LoadStatus getLoadStatus() const { return loadStatus; }
    void enableAllUnknownSets();
    void markAllSetsAsKnown();
    void notifyEnabledSetsChanged();

public slots:
    LoadStatus loadCardDatabases();
private slots:
    LoadStatus loadCardDatabase(const QString &path);
signals:
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();
    void cardDatabaseEnabledSetsChanged();
    void cardAdded(CardInfo *card);
    void cardRemoved(CardInfo *card);
};

#endif
