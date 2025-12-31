#ifndef CARD_INFO_H
#define CARD_INFO_H

#include "format/format_legality_rules.h"
#include "printing/printing_info.h"

#include <QDate>
#include <QHash>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QMetaType>
#include <QSharedPointer>
#include <QVariant>
#include <utility>

inline Q_LOGGING_CATEGORY(CardInfoLog, "card_info");

class CardInfo;
class CardSet;
class CardRelation;
class ICardDatabaseParser;

typedef QSharedPointer<CardInfo> CardInfoPtr;
typedef QSharedPointer<CardSet> CardSetPtr;
typedef QSharedPointer<FormatRules> FormatRulesPtr;
typedef QMap<QString, QList<PrintingInfo>> SetToPrintingsMap;

typedef QHash<QString, CardInfoPtr> CardNameMap;
typedef QHash<QString, CardSetPtr> SetNameMap;
typedef QHash<QString, FormatRulesPtr> FormatRulesNameMap;

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

public:
    /**
     * @class CardInfo::UiAttributes
     * @ingroup Cards
     *
     * @brief Attributes of the card that affect display and game logic.
     */
    struct UiAttributes
    {
        bool cipt = false;                 ///< Positioning flag used by UI.
        bool landscapeOrientation = false; ///< Orientation flag for rendering.
        int tableRow = 0;                  ///< Row index in a table or visual representation.
        bool upsideDownArt = false;        ///< Whether artwork is flipped for visual purposes.
    };

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
    UiAttributes uiAttributes;                     ///< Attributes that affect display and game logic
    QString setsNames;                             ///< Cached, human-readable list of set names.
    QSet<QString> altNames;                        ///< Cached set of alternate names, used when searching
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
     * @param _uiAttributes Attributes that affect display and game logic
     */
    explicit CardInfo(const QString &_name,
                      const QString &_text,
                      bool _isToken,
                      QVariantHash _properties,
                      const QList<CardRelation *> &_relatedCards,
                      const QList<CardRelation *> &_reverseRelatedCards,
                      SetToPrintingsMap _sets,
                      UiAttributes _uiAttributes);

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
          setsToPrintings(other.setsToPrintings), uiAttributes(other.uiAttributes), setsNames(other.setsNames),
          altNames(other.altNames)
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
     * @param _uiAttributes Attributes that affect display and game logic
     * @return Shared pointer to the new CardInfo instance.
     */
    static CardInfoPtr newInstance(const QString &_name,
                                   const QString &_text,
                                   bool _isToken,
                                   QVariantHash _properties,
                                   const QList<CardRelation *> &_relatedCards,
                                   const QList<CardRelation *> &_reverseRelatedCards,
                                   SetToPrintingsMap _sets,
                                   UiAttributes _uiAttributes);

    /**
     * @brief Clones the current CardInfo instance.
     *
     * Uses the copy constructor and ensures the smart pointer is properly set.
     *
     * @return Shared pointer to the cloned CardInfo.
     */
    [[nodiscard]] CardInfoPtr clone() const
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
    [[nodiscard]] inline const QString &getName() const
    {
        return name;
    }
    [[nodiscard]] const QString &getSimpleName() const
    {
        return simpleName;
    }
    const QSet<QString> &getAltNames()
    {
        return altNames;
    }
    [[nodiscard]] const QString &getText() const
    {
        return text;
    }
    void setText(const QString &_text)
    {
        text = _text;
        emit cardInfoChanged(smartThis);
    }
    [[nodiscard]] bool getIsToken() const
    {
        return isToken;
    }
    [[nodiscard]] QStringList getProperties() const
    {
        return properties.keys();
    }
    [[nodiscard]] QString getProperty(const QString &propertyName) const
    {
        return properties.value(propertyName).toString();
    }
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
        emit cardInfoChanged(smartThis);
    }
    [[nodiscard]] bool hasProperty(const QString &propertyName) const
    {
        return properties.contains(propertyName);
    }
    [[nodiscard]] const SetToPrintingsMap &getSets() const
    {
        return setsToPrintings;
    }
    [[nodiscard]] const QString &getSetsNames() const
    {
        return setsNames;
    }
    //@}

    /** @name Related Cards Accessors */ //@{
    [[nodiscard]] const QList<CardRelation *> &getRelatedCards() const
    {
        return relatedCards;
    }
    [[nodiscard]] const QList<CardRelation *> &getReverseRelatedCards() const
    {
        return reverseRelatedCards;
    }
    [[nodiscard]] const QList<CardRelation *> &getReverseRelatedCards2Me() const
    {
        return reverseRelatedCardsToMe;
    }
    [[nodiscard]] QList<CardRelation *> getAllRelatedCards() const
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
    [[nodiscard]] const UiAttributes &getUiAttributes() const
    {
        return uiAttributes;
    }
    //@}

    [[nodiscard]] QChar getColorChar() const;

    /** @name Legacy/Convenience Property Accessors */ //@{
    [[nodiscard]] QString getCardType() const;
    void setCardType(const QString &value);
    [[nodiscard]] QString getCmc() const;
    [[nodiscard]] QString getColors() const;
    void setColors(const QString &value);
    [[nodiscard]] QString getLoyalty() const;
    [[nodiscard]] QString getMainCardType() const;
    [[nodiscard]] QString getManaCost() const;
    [[nodiscard]] QString getPowTough() const;
    void setPowTough(const QString &value);
    //@}

    /**
     * @brief Returns a version of the card name safe for file storage or fuzzy matching.
     *
     * Removes invalid characters, replaces spacing markers, and normalizes diacritics.
     *
     * @return Corrected card name as a QString.
     */
    [[nodiscard]] QString getCorrectedName() const;

    /**
     * @brief Checks if the card is legal in the given format.
     * A card is considered legal in a format if its properties map contains an entry for "format-<name>", with value
     * "legal" or "restricted".
     * @param format The format's name. If empty, will always return true.
     * @return Whether the card is legal in the given format.
     */
    [[nodiscard]] bool isLegalInFormat(const QString &format) const;

    /**
     * @brief Adds a printing to a specific set.
     *
     * Updates the mapping and refreshes the cached list of set names.
     *
     * @param _set The set to which the card should be added.
     * @param _info Optional printing information.
     */
    void addToSet(const CardSetPtr &_set, const PrintingInfo &_info = PrintingInfo());

    /**
     * @brief Combines legality properties from a provided map.
     *
     * Useful for merging format legality flags from multiple sources.
     *
     * @param props Key-value mapping of format legalities.
     */
    void combineLegalities(const QVariantHash &props);

    /**
     * @brief Refreshes all cached fields that are calculated from the contained sets and printings.
     *
     * Typically called after adding or modifying set memberships or printings.
     */
    void refreshCachedSets();

    /**
     * @brief Simplifies a name for fuzzy matching.
     *
     * Converts to lowercase, removes punctuation/spacing.
     *
     * @param name Original name string.
     * @return Simplified name string.
     */
    static QString simplifyName(const QString &name);

private:
    /**
     * @brief Refreshes the cached, human-readable list of set names.
     *
     * Typically called after adding or modifying set memberships.
     */
    void refreshCachedSetNames();

    /**
     * @brief Refreshes the cached list of alt names for the card.
     *
     * Typically called after adding or modifying the contained printings.
     */
    void refreshCachedAltNames();

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
