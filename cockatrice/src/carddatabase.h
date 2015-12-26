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
};

class CardInfo : public QObject {
    Q_OBJECT
private:
    CardDatabase *db;

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
    bool upsideDownArt;
    int loyalty;
    QStringMap customPicURLs;
    MuidMap muIds;
    bool cipt;
    int tableRow;
    QString pixmapCacheKey;

public:
    CardInfo(CardDatabase *_db,
        const QString &_name = QString(),
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
        MuidMap muids = MuidMap()
        );
    ~CardInfo();
    const QString &getName() const { return name; }
    const QString &getSimpleName() const { return simpleName; }
    bool getIsToken() const { return isToken; }
    const SetList &getSets() const { return sets; }
    const QString &getManaCost() const { return manacost; }
    const QString &getCmc() const { return cmc; }
    const QString &getCardType() const { return cardtype; }
    const QString &getPowTough() const { return powtough; }
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
    const QStringList &getColors() const { return colors; }
    const QStringList &getRelatedCards() const { return relatedCards; }
    const QStringList &getReverseRelatedCards() const { return reverseRelatedCards; }
    const QStringList &getReverseRelatedCards2Me() const { return reverseRelatedCardsToMe; }
    void resetReverseRelatedCards2Me() { reverseRelatedCardsToMe = QStringList(); }
    void addReverseRelatedCards2Me(QString & cardName) { reverseRelatedCardsToMe.append(cardName); }
    bool getUpsideDownArt() const { return upsideDownArt; }
    QString getCustomPicURL(const QString &set) const { return customPicURLs.value(set); }
    int getMuId(const QString &set) const { return muIds.value(set); }
    QString getMainCardType() const;
    QString getCorrectedName() const;
    int getTableRow() const { return tableRow; }
    void setTableRow(int _tableRow) { tableRow = _tableRow; }
    void setLoyalty(int _loyalty) { loyalty = _loyalty; emit cardInfoChanged(this); }
    void setCustomPicURL(const QString &_set, const QString &_customPicURL) { customPicURLs.insert(_set, _customPicURL); }
    void setMuId(const QString &_set, const int &_muId) { muIds.insert(_set, _muId); }
    void addToSet(CardSet *set);
    void emitPixmapUpdated() { emit pixmapUpdated(); }

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

    /*
     * A dummy card returned by getCard() ad a fallback
     */
    CardInfo *noCard;

    LoadStatus loadStatus;
    bool detectedFirstRun;
private:
    static const int versionNeeded;
    void loadCardsFromXml(QXmlStreamReader &xml, bool tokens);
    void loadSetsFromXml(QXmlStreamReader &xml);

    CardInfo *getCardFromMap(CardNameMap &cardMap, const QString &cardName, bool createIfNotFound);
    void checkUnknownSets();
public:
    static const char* TOKENS_SETNAME;

    CardDatabase(QObject *parent = 0);
    ~CardDatabase();
    void clear();
    void addCard(CardInfo *card);
    void removeCard(CardInfo *card);
    /*
     * Get card object by name. Ensured to return a valid CardInfo * object; check noCard
     */
    CardInfo *getCard(const QString &cardName = QString(), bool createIfNotFound = false);
    QList <CardInfo *> getCards(const QStringList &cardNames);

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    CardInfo *getCardBySimpleName(const QString &cardName = QString(), bool createIfNotFound = false);

    CardSet *getSet(const QString &setName);
    QList<CardInfo *> getCardList() const { return cards.values(); }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName, bool tokens = false);
    bool saveToFile(const QString &fileName, bool tokens = false);
    QStringList getAllColors() const;
    QStringList getAllMainCardTypes() const;
    LoadStatus getLoadStatus() const { return loadStatus; }
    bool getLoadSuccess() const { return loadStatus == Ok; }
    bool hasDetectedFirstRun();
    void refreshCachedReverseRelatedCards();
public slots:
    LoadStatus loadCardDatabase(const QString &path, bool tokens = false);
    void loadCustomCardDatabases(const QString &path);
    void emitCardListChanged();
private slots:
    void loadCardDatabase();
    void loadTokenDatabase();
signals:
    void cardListChanged();
    void cardAdded(CardInfo *card);
    void cardRemoved(CardInfo *card);
};

#endif
