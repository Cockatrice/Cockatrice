#ifndef CARD_ART_UTILS_H
#define CARD_ART_UTILS_H

#include <QPixmap>

class ExactCard;

namespace CardArtUtils
{
/**
 * @brief Rotates a card's art upright when its layout shows sideways.
 *
 * Sideways-layout cards (planes, sieges/battles, split cards) store their
 * landscape artwork rotated 90° inside a portrait frame. Art-crop displays,
 * playmat art, and the card-info picture must show such art upright before
 * sampling or painting. Portrait cards are returned unchanged.
 *
 * @param art The card pixmap to orient.
 * @param card The card describing the art orientation.
 * @return @p art rotated 90° clockwise for sideways-layout cards, else @p art.
 */
QPixmap rotateSidewaysLayoutArt(const QPixmap &art, const ExactCard &card);

/**
 * Return the upright artwork used by banner previews, user rows and profile
 * popups. The crop excludes conventional name and rules boxes before callers apply
 * their saved pan and zoom, so a taller popup cannot expose the card frame.
 */
QPixmap cropCardArt(const QPixmap &art, const ExactCard &card);
} // namespace CardArtUtils

#endif // CARD_ART_UTILS_H
