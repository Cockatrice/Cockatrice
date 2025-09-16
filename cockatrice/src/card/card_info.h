#ifndef CARD_INFO_H
#define CARD_INFO_H

#include <QDate>
#include <QHash>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QMetaType>
#include <QSharedPointer>
#include <QStringList>
#include <QVariant>
#include <utility>

inline Q_LOGGING_CATEGORY(CardInfoLog, "card_info");

class CardInfo;
class PrintingInfo;
class CardSet;
class CardRelation;
class ICardDatabaseParser;

typedef QSharedPointer<CardInfo> CardInfoPtr;
typedef QSharedPointer<CardSet> CardSetPtr;
typedef QMap<QString, QList<PrintingInfo>> SetToPrintingsMap;

typedef QHash<QString, CardInfoPtr> CardNameMap;
typedef QHash<QString, CardSetPtr> SetNameMap;

Q_DECLARE_METATYPE(CardInfoPtr)

class CardSet : public QList<CardInfoPtr>
{
public:
    enum Priority
    {
        PriorityFallback = 0,
        PriorityPrimary = 10,
        PrioritySecondary = 20,
        PriorityReprint = 30,
        PriorityOther = 40,
        PriorityLowest = 100,
    };

    static const char *TOKENS_SETNAME;

private:
    QString shortName, longName;
    unsigned int sortKey;
    QDate releaseDate;
    QString setType;
    Priority priority;
    bool enabled, isknown;

public:
    explicit CardSet(const QString &_shortName = QString(),
                     const QString &_longName = QString(),
                     const QString &_setType = QString(),
                     const QDate &_releaseDate = QDate(),
                     const Priority _priority = PriorityFallback);
    static CardSetPtr newInstance(const QString &_shortName = QString(),
                                  const QString &_longName = QString(),
                                  const QString &_setType = QString(),
                                  const QDate &_releaseDate = QDate(),
                                  const Priority _priority = PriorityFallback);
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
    Priority getPriority() const
    {
        return priority;
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
    void setPriority(const Priority _priority)
    {
        priority = _priority;
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
    void defaultSort();
};

/**
 * Info relating to a specific printing for a card.
 */
class PrintingInfo
{
public:
    explicit PrintingInfo(const CardSetPtr &_set = nullptr);
    ~PrintingInfo() = default;

    bool operator==(const PrintingInfo &other) const
    {
        return this->set == other.set && this->properties == other.properties;
    }

private:
    CardSetPtr set;
    // per-printing card properties;
    QVariantHash properties;

public:
    CardSetPtr getSet() const
    {
        return set;
    }
    QStringList getProperties() const
    {
        return properties.keys();
    }
    QString getProperty(const QString &propertyName) const
    {
        return properties.value(propertyName).toString();
    }
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
    }

    QString getUuid() const;
};

class CardInfo : public QObject
{
    Q_OBJECT
private:
    CardInfoPtr smartThis;
    // The card name
    QString name;
    // The name without punctuation or capitalization, for better card name recognition.
    QString simpleName;
    // card text
    QString text;
    // whether this is not a "real" card but a token
    bool isToken;
    // basic card properties; common for all the sets
    QVariantHash properties;
    // the cards i'm related to
    QList<CardRelation *> relatedCards;
    // the card i'm reverse-related to
    QList<CardRelation *> reverseRelatedCards;
    // the cards thare are reverse-related to me
    QList<CardRelation *> reverseRelatedCardsToMe;
    // card sets
    SetToPrintingsMap setsToPrintings;
    // cached set names
    QString setsNames;
    // positioning properties; used by UI
    bool cipt;
    bool landscapeOrientation;
    int tableRow;
    bool upsideDownArt;

public:
    explicit CardInfo(const QString &_name,
                      const QString &_text,
                      bool _isToken,
                      QVariantHash _properties,
                      const QList<CardRelation *> &_relatedCards,
                      const QList<CardRelation *> &_reverseRelatedCards,
                      SetToPrintingsMap _sets,
                      bool _cipt,
                      bool _landscapeOrientation,
                      int _tableRow,
                      bool _upsideDownArt);
    CardInfo(const CardInfo &other)
        : QObject(other.parent()), name(other.name), simpleName(other.simpleName), text(other.text),
          isToken(other.isToken), properties(other.properties), relatedCards(other.relatedCards),
          reverseRelatedCards(other.reverseRelatedCards), reverseRelatedCardsToMe(other.reverseRelatedCardsToMe),
          setsToPrintings(other.setsToPrintings), setsNames(other.setsNames), cipt(other.cipt),
          landscapeOrientation(other.landscapeOrientation), tableRow(other.tableRow), upsideDownArt(other.upsideDownArt)
    {
    }

    static CardInfoPtr newInstance(const QString &_name);

    static CardInfoPtr newInstance(const QString &_name,
                                   const QString &_text,
                                   bool _isToken,
                                   QVariantHash _properties,
                                   const QList<CardRelation *> &_relatedCards,
                                   const QList<CardRelation *> &_reverseRelatedCards,
                                   SetToPrintingsMap _sets,
                                   bool _cipt,
                                   bool _landscapeOrientation,
                                   int _tableRow,
                                   bool _upsideDownArt);

    CardInfoPtr clone() const
    {
        // Use the copy constructor to create a new instance
        CardInfoPtr newCardInfo = CardInfoPtr(new CardInfo(*this));
        newCardInfo->setSmartPointer(newCardInfo); // Set the smart pointer for the new instance
        return newCardInfo;
    }

    void setSmartPointer(CardInfoPtr _ptr)
    {
        smartThis = std::move(_ptr);
    }

    // basic properties
    inline const QString &getName() const
    {
        return name;
    }
    const QString &getSimpleName() const
    {
        return simpleName;
    }

    const QString &getText() const
    {
        return text;
    }
    void setText(const QString &_text)
    {
        text = _text;
        emit cardInfoChanged(smartThis);
    }

    bool getIsToken() const
    {
        return isToken;
    }
    QStringList getProperties() const
    {
        return properties.keys();
    }
    QString getProperty(const QString &propertyName) const
    {
        return properties.value(propertyName).toString();
    }
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
        emit cardInfoChanged(smartThis);
    }
    bool hasProperty(const QString &propertyName) const
    {
        return properties.contains(propertyName);
    }
    const SetToPrintingsMap &getSets() const
    {
        return setsToPrintings;
    }
    const QString &getSetsNames() const
    {
        return setsNames;
    }

    // related cards
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
    QList<CardRelation *> getAllRelatedCards() const
    {
        QList<CardRelation *> result;
        result.append(getRelatedCards());
        result.append(getReverseRelatedCards2Me());
        return result;
    }
    void resetReverseRelatedCards2Me();
    void addReverseRelatedCards2Me(CardRelation *cardRelation)
    {
        reverseRelatedCardsToMe.append(cardRelation);
    }

    // positioning
    bool getCipt() const
    {
        return cipt;
    }
    bool getLandscapeOrientation() const
    {
        return landscapeOrientation;
    }
    int getTableRow() const
    {
        return tableRow;
    }
    void setTableRow(int _tableRow)
    {
        tableRow = _tableRow;
    }
    bool getUpsideDownArt() const
    {
        return upsideDownArt;
    }
    const QChar getColorChar() const;

    // Back-compatibility methods. Remove ASAP
    const QString getCardType() const;
    void setCardType(const QString &value);
    const QString getCmc() const;
    const QString getColors() const;
    void setColors(const QString &value);
    const QString getLoyalty() const;
    const QString getMainCardType() const;
    const QString getManaCost() const;
    const QString getPowTough() const;
    void setPowTough(const QString &value);

    QString getCorrectedName() const;
    void addToSet(const CardSetPtr &_set, PrintingInfo _info = PrintingInfo());
    void combineLegalities(const QVariantHash &props);
    void refreshCachedSetNames();

    /**
     * Simplify a name to have no punctuation and lowercase all letters, for
     * less strict name-matching.
     */
    static QString simplifyName(const QString &name);

signals:
    /**
     * Emit this when a pixmap for this card finishes loading.
     * @param printing The specific printing the pixmap is for.
     */
    void pixmapUpdated(const PrintingInfo &printing);
    void cardInfoChanged(CardInfoPtr card);
};

class CardRelation : public QObject
{
    Q_OBJECT
public:
    enum AttachType
    {
        DoesNotAttach = 0,
        AttachTo = 1,
        TransformInto = 2,
    };

private:
    QString name;
    AttachType attachType;
    bool isCreateAllExclusion;
    bool isVariableCount;
    int defaultCount;
    bool isPersistent;

public:
    explicit CardRelation(const QString &_name = QString(),
                          AttachType _attachType = DoesNotAttach,
                          bool _isCreateAllExclusion = false,
                          bool _isVariableCount = false,
                          int _defaultCount = 1,
                          bool _isPersistent = false);

    inline const QString &getName() const
    {
        return name;
    }
    AttachType getAttachType() const
    {
        return attachType;
    }
    bool getDoesAttach() const
    {
        return attachType != DoesNotAttach;
    }
    bool getDoesTransform() const
    {
        return attachType == TransformInto;
    }
    QString getAttachTypeAsString() const
    {
        switch (attachType) {
            case AttachTo:
                return "attach";
            case TransformInto:
                return "transform";
            default:
                return "";
        }
    }
    bool getCanCreateAnother() const
    {
        return !getDoesAttach();
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
    bool getIsPersistent() const
    {
        return isPersistent;
    }
};
#endif
