#ifndef COCKATRICE_CARD_SET_H
#define COCKATRICE_CARD_SET_H

#include <QDate>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>

class CardInfo;
using CardInfoPtr = QSharedPointer<CardInfo>;

class CardSet;
using CardSetPtr = QSharedPointer<CardSet>;

/**
 * @class CardSet
 * @ingroup CardSets
 *
 * @brief A collection of cards grouped under a common identifier.
 *
 * A set serves both as metadata (identifier, title, category, release date, and priority)
 * and as a container of all cards that belong to it. Each set can be enabled/disabled
 * and marked as known/unknown depending on context.
 *
 * The class inherits from `QList<CardInfoPtr>`, so it can be iterated over directly
 * to access its contents.
 *
 * Typical usage:
 * - Query metadata such as identifier, category, or release date.
 * - Enable or disable sets according to user preference.
 * - Store and retrieve CardInfo objects associated with the set.
 */
class CardSet : public QList<CardInfoPtr>
{
public:
    /**
     * @enum Priority
     * @brief Defines relative ordering and importance of sets.
     */
    enum Priority
    {
        PriorityFallback = 0,   ///< Used when no other priority is defined.
        PriorityPrimary = 10,   ///< Primary, canonical set.
        PrioritySecondary = 20, ///< Secondary but relevant.
        PriorityReprint = 30,   ///< Duplicate or reprint category.
        PriorityOther = 40,     ///< Miscellaneous grouping.
        PriorityLowest = 100,   ///< Lowest sorting priority.
    };

    static const char *TOKENS_SETNAME; ///< Reserved identifier for token-like sets.

private:
    ICardSetPriorityController *priorityController; ///< Interface to the card set priority controller.
    QString shortName;                              ///< Short identifier for the set.
    QString longName;                               ///< Full name for the set.
    unsigned int sortKey;                           ///< Custom numeric sort key.
    QDate releaseDate;                              ///< Release date, may be empty if unknown.
    QString setType;                                ///< Type/category label for the set.
    Priority priority;                              ///< Priority level for sorting and relevance.
    bool enabled;                                   ///< Whether the set is active/enabled.
    bool isknown;                                   ///< Whether the set is considered known.

public:
    /**
     * @brief Constructs a CardSet.
     *
     * @param priorityController Interface to a card set priority controller.
     * @param _shortName Identifier string.
     * @param _longName Full descriptive name.
     * @param _setType Type/category string.
     * @param _releaseDate Release date (optional).
     * @param _priority Sorting/priority level.
     */
    explicit CardSet(ICardSetPriorityController *priorityController,
                     const QString &_shortName = QString(),
                     const QString &_longName = QString(),
                     const QString &_setType = QString(),
                     const QDate &_releaseDate = QDate(),
                     const Priority _priority = PriorityFallback);

    /**
     * @brief Creates and returns a new shared CardSet instance.
     *
     * @param priorityController Interface to a card set priority controller.
     * @param _shortName Identifier string.
     * @param _longName Full descriptive name.
     * @param _setType Type/category string.
     * @param _releaseDate Release date (optional).
     * @param _priority Sorting/priority level.
     * @return A shared pointer to the new CardSet.
     */
    static CardSetPtr newInstance(ICardSetPriorityController *priorityController,
                                  const QString &_shortName = QString(),
                                  const QString &_longName = QString(),
                                  const QString &_setType = QString(),
                                  const QDate &_releaseDate = QDate(),
                                  const Priority _priority = PriorityFallback);

    /**
     * @brief Returns a safe, sanitized version of the short name.
     *
     * Intended for file paths or identifiers where only certain
     * characters are allowed.
     *
     * @return Sanitized short name.
     */
    QString getCorrectedShortName() const;

    /// @return Short identifier of the set.
    QString getShortName() const
    {
        return shortName;
    }

    /// @return Descriptive name of the set.
    QString getLongName() const
    {
        return longName;
    }

    /// @return Type/category string of the set.
    QString getSetType() const
    {
        return setType;
    }

    /// @return Release date of the set.
    QDate getReleaseDate() const
    {
        return releaseDate;
    }

    /// @return Priority level of the set.
    Priority getPriority() const
    {
        return priority;
    }

    /**
     * @brief Sets the full name of the set.
     * @param _longName New full name.
     */
    void setLongName(const QString &_longName)
    {
        longName = _longName;
    }

    /**
     * @brief Sets the category/type of the set.
     * @param _setType New category string.
     */
    void setSetType(const QString &_setType)
    {
        setType = _setType;
    }

    /**
     * @brief Sets the release date of the set.
     * @param _releaseDate New release date.
     */
    void setReleaseDate(const QDate &_releaseDate)
    {
        releaseDate = _releaseDate;
    }

    /**
     * @brief Updates the priority of the set.
     * @param _priority New priority value.
     */

    void setPriority(const Priority _priority)
    {
        priority = _priority;
    }

    /**
     * @brief Loads state values (enabled, known, sort key) from configuration.
     *
     * Reads external configuration and applies it to this set.
     */
    void loadSetOptions();

    /// @return The sort key assigned to this set.
    int getSortKey() const
    {
        return sortKey;
    }

    /**
     * @brief Assigns a new sort key to this set.
     * @param _sortKey The numeric key to use for sorting.
     */
    void setSortKey(unsigned int _sortKey);

    /// @return True if the set is enabled.
    bool getEnabled() const
    {
        return enabled;
    }

    /**
     * @brief Enables or disables the set.
     * @param _enabled True to enable, false to disable.
     */
    void setEnabled(bool _enabled);

    /// @return True if the set is considered known.
    bool getIsKnown() const
    {
        return isknown;
    }

    /**
     * @brief Marks the set as known or unknown.
     * @param _isknown True if known, false if unknown.
     */
    void setIsKnown(bool _isknown);

    /**
     * @brief Determines whether the set has incomplete metadata and should be ignored.
     *
     * @return True if the long name, type, and release date are all empty.
     */
    bool getIsKnownIgnored() const
    {
        return longName.length() + setType.length() + releaseDate.toString().length() == 0;
    }
};

#endif // COCKATRICE_CARD_SET_H
