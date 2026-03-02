#ifndef EXACT_CARD_H
#define EXACT_CARD_H

#include "../card_info.h"

/**
 * @class ExactCard
 * @ingroup CardPrintings
 *
 * @brief Represents a specific card instance, defined by its CardInfo
 *        and a particular printing.
 *
 * An ExactCard identifies a card not only by its underlying CardInfoPtr
 * (which may be null), but also by its PrintingInfo, which specifies the
 * exact printing/variant. This allows distinguishing between different
 * printings of the same logical card (e.g., different sets, promos, foils).
 */
class ExactCard
{
    CardInfoPtr card;
    PrintingInfo printing;

public:
    /**
     * @brief Constructs an empty ExactCard.
     *
     * The CardInfoPtr will be null, and PrintingInfo will be default-constructed.
     * An empty ExactCard represents "no card".
     */
    ExactCard();

    /**
     * @brief Constructs an ExactCard from a card and printing.
     *
     * @param _card     The card info pointer. May be null.
     * @param _printing The printing details. Defaults to an empty PrintingInfo.
     */
    explicit ExactCard(const CardInfoPtr &_card, const PrintingInfo &_printing = PrintingInfo());

    /**
     * @brief Returns the underlying CardInfoPtr.
     *
     * May be null if the ExactCard is empty.
     */
    [[nodiscard]] CardInfoPtr getCardPtr() const
    {
        return card;
    }

    /**
     * @brief Returns the printing information associated with this card.
     *
     * May be empty if no specific printing was assigned.
     */
    [[nodiscard]] PrintingInfo getPrinting() const
    {
        return printing;
    }

    /**
     * @brief Compares both card pointer and printing for equality.
     *
     * Two ExactCard objects are equal only if both their CardInfoPtr and
     * PrintingInfo values are equal.
     */
    bool operator==(const ExactCard &other) const;

    /**
     * @brief Convenience helper to get the card's display name.
     *
     * @return The card's name, or an empty string if the CardInfoPtr is null.
     */
    [[nodiscard]] QString getName() const;

    /**
     * @brief Returns a reference to the underlying CardInfo object.
     *
     * If the CardInfoPtr is null, returns a reference to a static empty CardInfo
     * instance instead. This avoids null-dereferencing but means modifications
     * to the returned object do not affect the ExactCard.
     *
     * @return A const reference to the CardInfo object.
     */
    [[nodiscard]] const CardInfo &getInfo() const;

    /**
     * @brief Generates a stable cache key for pixmap caching.
     *
     * The key includes the card's name and (if present) the printing UUID,
     * allowing different printings of the same card to map to different cache entries.
     */
    [[nodiscard]] QString getPixmapCacheKey() const;

    /**
     * @brief Indicates whether this ExactCard represents no valid card.
     *
     * An ExactCard is considered empty if the CardInfoPtr is null or the
     * card's name is empty.
     */
    [[nodiscard]] bool isEmpty() const;

    /**
     * @brief Boolean conversion indicating whether the card is valid (non-empty).
     *
     * @return true if not empty, false otherwise.
     */
    explicit operator bool() const;

    /**
     * @brief Emits the pixmapUpdated signal on the underlying CardInfo.
     *
     * Assumes CardInfoPtr is non-null. If called on an empty ExactCard,
     * the behavior is undefined.
     */
    void emitPixmapUpdated() const;
};

#endif // EXACT_CARD_H
