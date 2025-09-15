#include "exact_card.h"

/**
 * Default constructor.
 * This will set the CardInfoPtr to null.
 * The printing will be the default-constructed PrintingInfo.
 */
ExactCard::ExactCard()
{
}

/**
 * @param _card The card. Can be null.
 * @param _printing The printing. Can be empty.
 */
ExactCard::ExactCard(const CardInfoPtr &_card, const PrintingInfo &_printing) : card(_card), printing(_printing)
{
}

bool ExactCard::operator==(const ExactCard &other) const
{
    return this->card == other.card && this->printing == other.printing;
}

/**
 * Convenience method to safely get the card's name.
 * @return The name in the CardInfo, or an empty string if card is null
 */
QString ExactCard::getName() const
{
    return card.isNull() ? "" : card->getName();
}

/**
 * Gets a view of the underlying cardInfoPtr.
 * @return A const reference to the CardInfo, or an empty CardInfo if card is null
 */
const CardInfo &ExactCard::getInfo() const
{
    if (card.isNull()) {
        static CardInfoPtr emptyCard = CardInfo::newInstance("");
        return *emptyCard;
    }
    return *card;
}

/**
 * The key used to identify this exact printing in the cache
 */
QString ExactCard::getPixmapCacheKey() const
{
    QString uuid = printing.getUuid();
    QString suffix = uuid.isEmpty() ? "" : "_" + uuid;
    return QLatin1String("card_") + card->getName() + suffix;
}

/**
 * Checks if the card is null or empty.
 */
bool ExactCard::isEmpty() const
{
    return card.isNull() || card->getName().isEmpty();
}

/**
 * Returns true if isEmpty() is false
 */
ExactCard::operator bool() const
{
    return !isEmpty();
}

/**
 * Gets the CardInfo to emit the pixmapUpdated signal
 */
void ExactCard::emitPixmapUpdated() const
{
    emit card->pixmapUpdated(printing);
}