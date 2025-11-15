#ifndef CARD_INFO_H
#define CARD_INFO_H

#include "printing/printing_info.h"

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

/**
 * @class CardInfo
 * @ingroup Cards
 *
 * @brief Represents a card and its associated metadata, properties, and relationships.
 *
 * CardInfo holds both static information (name, text, flags) and dynamic data
 * (properties, set memberships, relationships). It also integrates with
 * signals/slots, allowing observers to react to property or visual updates.
 *
 * Each CardInfo may belong to multiple sets through its printings, and can
 * be related to other cards through defined relationships.
 */
class CardInfo : public QObject
{
    Q_OBJECT

private:
    /** @name Private Card Properties
     *  @anchor PrivateCardProperties
     */
    ///@{
    CardInfoPtr smartThis;                         ///< Smart pointer to self for safe cross-references.
    QString name;                                  ///< Full name of the card.
    QString simpleName;                            ///< Simplified name for fuzzy matching.
    QString text;                                  ///< Text description or rules text of the card.
    bool isToken;                                  ///< Whether this card is a token or not.
    QVariantHash properties;                       ///< Key-value store of dynamic card properties.
    QList<CardRelation *> relatedCards;            ///< Forward references to related cards.
    QList<CardRelation *> reverseRelatedCards;     ///< Cards that refer back to this card.
    QList<CardRelation *> reverseRelatedCardsToMe; ///< Cards that consider this card as related.
    SetToPrintingsMap setsToPrintings;             ///< Mapping from set names to printing variations.
    QString setsNames;                             ///< Cached, human-readable list of set names.
    bool cipt;                                     ///< Positioning flag used by UI.
    bool landscapeOrientation;                     ///< Orientation flag for rendering.
    int tableRow;                                  ///< Row index in a table or visual representation.
    bool upsideDownArt;                            ///< Whether artwork is flipped for visual purposes.
    ///@}

public:
    /**
     * @brief Constructs a CardInfo with full initialization.
     *
     * @param _name The name of the card.
     * @param _text Rules text or description of the card.
     * @param _isToken Flag indicating whether the card is a token.
     * @param _properties Arbitrary key-value properties.
     * @param _relatedCards Forward references to related cards.
     * @param _reverseRelatedCards Backward references to related cards.
     * @param _sets Map of set names to printing information.
     * @param _cipt UI positioning flag.
     * @param _landscapeOrientation UI rendering orientation.
     * @param _tableRow Row index for table placement.
     * @param _upsideDownArt Whether the artwork should be displayed upside down.
     */
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

    /**
     * @brief Copy constructor for CardInfo.
     *
     * Performs a deep copy of properties, sets, and related card lists.
     *
     * @param other Another CardInfo to copy.
     */
    CardInfo(const CardInfo &other)
        : QObject(other.parent()), name(other.name), simpleName(other.simpleName), text(other.text),
          isToken(other.isToken), properties(other.properties), relatedCards(other.relatedCards),
          reverseRelatedCards(other.reverseRelatedCards), reverseRelatedCardsToMe(other.reverseRelatedCardsToMe),
          setsToPrintings(other.setsToPrintings), setsNames(other.setsNames), cipt(other.cipt),
          landscapeOrientation(other.landscapeOrientation), tableRow(other.tableRow), upsideDownArt(other.upsideDownArt)
    {
    }

    /**
     * @brief Creates a new instance with only the card name.
     *
     * All other fields are set to defaults.
     *
     * @param _name The card name.
     * @return Shared pointer to the new CardInfo instance.
     */
    static CardInfoPtr newInstance(const QString &_name);

    /**
     * @brief Creates a new instance with full initialization.
     *
     * @param _name Name of the card.
     * @param _text Rules text or description.
     * @param _isToken Token flag.
     * @param _properties Arbitrary properties.
     * @param _relatedCards Forward relationships.
     * @param _reverseRelatedCards Reverse relationships.
     * @param _sets Printing information per set.
     * @param _cipt UI positioning flag.
     * @param _landscapeOrientation UI rendering orientation.
     * @param _tableRow Row index for table placement.
     * @param _upsideDownArt Artwork orientation flag.
     * @return Shared pointer to the new CardInfo instance.
     */
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

    /**
     * @brief Clones the current CardInfo instance.
     *
     * Uses the copy constructor and ensures the smart pointer is properly set.
     *
     * @return Shared pointer to the cloned CardInfo.
     */
    CardInfoPtr clone() const
    {
        auto newCardInfo = CardInfoPtr(new CardInfo(*this));
        newCardInfo->setSmartPointer(newCardInfo); // Set the smart pointer for the new instance
        return newCardInfo;
    }

    /**
     * @brief Sets the internal smart pointer to self.
     *
     * Used internally to allow safe cross-references among CardInfo and CardSet.
     *
     * @param _ptr Shared pointer pointing to this instance.
     */
    void setSmartPointer(CardInfoPtr _ptr)
    {
        smartThis = std::move(_ptr);
    }

    /** @name Basic Properties Accessors */ //@{
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
    //@}

    /** @name Related Cards Accessors */ //@{
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
    //@}

    /** @name UI Positioning */ //@{
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
    //@}

    /** @name Legacy/Convenience Property Accessors */ //@{
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
    //@}

    /**
     * @brief Returns a version of the card name safe for file storage or fuzzy matching.
     *
     * Removes invalid characters, replaces spacing markers, and normalizes diacritics.
     *
     * @return Corrected card name as a QString.
     */
    QString getCorrectedName() const;

    /**
     * @brief Adds a printing to a specific set.
     *
     * Updates the mapping and refreshes the cached list of set names.
     *
     * @param _set The set to which the card should be added.
     * @param _info Optional printing information.
     */
    void addToSet(const CardSetPtr &_set, PrintingInfo _info = PrintingInfo());

    /**
     * @brief Combines legality properties from a provided map.
     *
     * Useful for merging format legality flags from multiple sources.
     *
     * @param props Key-value mapping of format legalities.
     */
    void combineLegalities(const QVariantHash &props);

    /**
     * @brief Refreshes the cached, human-readable list of set names.
     *
     * Typically called after adding or modifying set memberships.
     */
    void refreshCachedSetNames();

    /**
     * @brief Simplifies a name for fuzzy matching.
     *
     * Converts to lowercase, removes punctuation/spacing.
     *
     * @param name Original name string.
     * @return Simplified name string.
     */
    static QString simplifyName(const QString &name);

signals:
    /**
     * @brief Emitted when a pixmap for this card has been updated or finished loading.
     *
     * @param printing Specific printing for which the pixmap has updated.
     */
    void pixmapUpdated(const PrintingInfo &printing);

    /**
     * @brief Emitted when card properties or state have changed.
     *
     * @param card Shared pointer to the CardInfo instance that changed.
     */
    void cardInfoChanged(CardInfoPtr card);
};
#endif
