/**
 * @file card_info.h
 * @ingroup Cards
 * @brief TODO: Document this.
 */

#ifndef CARD_INFO_H
#define CARD_INFO_H

#include "printing_info.h"

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
class CardSet;
class CardRelation;
class ICardDatabaseParser;

typedef QSharedPointer<CardInfo> CardInfoPtr;
typedef QSharedPointer<CardSet> CardSetPtr;
typedef QMap<QString, QList<PrintingInfo>> SetToPrintingsMap;

typedef QHash<QString, CardInfoPtr> CardNameMap;
typedef QHash<QString, CardSetPtr> SetNameMap;

Q_DECLARE_METATYPE(CardInfoPtr)


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
#endif
