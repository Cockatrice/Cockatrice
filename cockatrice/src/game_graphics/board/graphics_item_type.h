/**
 * @file graphics_item_type.h
 * @ingroup GameGraphics
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_GRAPHICS_ITEM_TYPE_H
#define COCKATRICE_GRAPHICS_ITEM_TYPE_H

#include <QGraphicsItem>

enum GraphicsItemType
{
    typeCard = QGraphicsItem::UserType + 1,
    typeCardDrag = QGraphicsItem::UserType + 2,
    typeZone = QGraphicsItem::UserType + 3,
    typePlayerTarget = QGraphicsItem::UserType + 4,
    typeDeckViewCardContainer = QGraphicsItem::UserType + 5,
    typeOther = QGraphicsItem::UserType + 6
};

#endif // COCKATRICE_GRAPHICS_ITEM_TYPE_H
