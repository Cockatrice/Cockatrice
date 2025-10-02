#ifndef EXACT_CARD_H
#define EXACT_CARD_H

#include "../card_info.h"

/**
 * @class ExactCard
 * @ingroup Cards
 * @brief Identifies the card by its CardInfoPtr along with its exact printing by its PrintingInfo.
 */
class ExactCard
{
    CardInfoPtr card;
    PrintingInfo printing;

public:
    ExactCard();
    explicit ExactCard(const CardInfoPtr &_card, const PrintingInfo &_printing = PrintingInfo());

    /**
     * Gets the CardInfoPtr. Can be null.
     */
    CardInfoPtr getCardPtr() const
    {
        return card;
    }

    /**
     * Gets the PrintingInfo. Can be empty.
     */
    PrintingInfo getPrinting() const
    {
        return printing;
    }

    bool operator==(const ExactCard &other) const;

    QString getName() const;
    const CardInfo &getInfo() const;
    QString getPixmapCacheKey() const;

    bool isEmpty() const;
    explicit operator bool() const;

    void emitPixmapUpdated() const;
};

#endif // EXACT_CARD_H
