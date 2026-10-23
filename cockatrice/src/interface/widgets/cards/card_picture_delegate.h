/**
 * @file card_picture_delegate.h
 * @ingroup CardWidgets
 * @brief Paints a card picture for a visual card grid item.
 */

#ifndef CARD_PICTURE_DELEGATE_H
#define CARD_PICTURE_DELEGATE_H

#include <QSize>
#include <QStyledItemDelegate>
#include <qtmetamacros.h>

class CardDatabaseVisualDisplayModel;

/**
 * @brief Draws the card of a CardDatabaseVisualDisplayModel row as a picture.
 *
 * The delegate only paints rows the view asks for, so a grid of the whole card
 * database stays cheap: no widget per card and no pixmap loaded for items that
 * are off screen. Card images come from CardPictureLoader, which caches them and
 * loads missing ones asynchronously.
 */
class CardPictureDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// Gap left around a card picture inside its grid cell, in pixels.
    static constexpr int SPACING = 4;

    explicit CardPictureDelegate(CardDatabaseVisualDisplayModel *_model, QObject *parent = nullptr);

    /// The picture size inside a grid cell. The cell is this size plus SPACING.
    void setCardSize(const QSize &size);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    CardDatabaseVisualDisplayModel *model;
    QSize cardSize;
};

#endif // CARD_PICTURE_DELEGATE_H
