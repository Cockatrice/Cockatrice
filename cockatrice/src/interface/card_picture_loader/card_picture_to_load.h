#ifndef PICTURE_TO_LOAD_H
#define PICTURE_TO_LOAD_H

#include <libcockatrice/card/printing/exact_card.h>

inline Q_LOGGING_CATEGORY(CardPictureToLoadLog, "card_picture_loader.picture_to_load");

/**
 * @class CardPictureToLoad
 * @ingroup PictureLoader
 * @brief Manages all URLs and sets for downloading a specific card image.
 *
 * Responsibilities:
 * - Maintains a sorted list of sets for a card.
 * - Generates URLs to download images from, including custom URLs and URL templates.
 * - Tracks which URL and set is currently being attempted.
 * - Provides helper methods to advance to next URL or set.
 */
class CardPictureToLoad
{
private:
    ExactCard card;                ///< The ExactCard being downloaded
    QList<CardSetPtr> sortedSets;  ///< All sets for this card, sorted by priority
    QList<QString> urlTemplates;   ///< URL templates from settings
    QList<QString> currentSetUrls; ///< URLs for the current set being attempted
    QString currentUrl;            ///< Currently active URL to download
    CardSetPtr currentSet;         ///< Currently active set

public:
    /**
     * @brief Constructs a CardPictureToLoad for a given ExactCard.
     * @param _card The card to download
     *
     * Initializes URL templates and pre-populates the first set URLs.
     */
    explicit CardPictureToLoad(const ExactCard &_card);

    /** @return The card being loaded. */
    [[nodiscard]] const ExactCard &getCard() const
    {
        return card;
    }

    /** @return The current URL being attempted. */
    [[nodiscard]] QString getCurrentUrl() const
    {
        return currentUrl;
    }

    /** @return The current set being attempted. */
    [[nodiscard]] CardSetPtr getCurrentSet() const
    {
        return currentSet;
    }

    /** @return The short name of the current set, or empty string if no set. */
    [[nodiscard]] QString getSetName() const;

    /**
     * @brief Transforms a URL template into a concrete URL for this card/set.
     * @param urlTemplate The URL template to transform
     * @return The transformed URL or empty string if the template cannot be fulfilled
     */
    QString transformUrl(const QString &urlTemplate) const;

    /**
     * @brief Advance to the next set in the list.
     * @return True if a next set exists and was selected, false if at the end.
     *
     * Updates currentSet and repopulates currentSetUrls.
     * If we are already at the end of the list, then currentSet is set to empty.
     */
    bool nextSet();

    /**
     * @brief Advance to the next URL in the current set's list.
     * @return True if a next URL exists, false if at the end.
     *
     * Updates currentUrl.
     * If we are already at the end of the list, then currentUrl is set to empty.
     */
    bool nextUrl();

    /**
     * @brief Populates the currentSetUrls list with URLs for the current set.
     *
     * Includes custom URLs first, followed by template-based URLs.
     */
    void populateSetUrls();

    /**
     * @brief Extract all sets from the card and sort them by priority.
     * @param card The card to extract sets from
     * @return A non-empty list of CardSetPtr, sorted by priority
     *
     * If the card has no sets, a dummy set is inserted. Also ensures
     * the printing corresponding to the ExactCard is first in the list.
     */
    static QList<CardSetPtr> extractSetsSorted(const ExactCard &card);
};

#endif // PICTURE_TO_LOAD_H
