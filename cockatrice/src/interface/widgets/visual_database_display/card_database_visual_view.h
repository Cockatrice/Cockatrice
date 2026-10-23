/**
 * @file card_database_visual_view.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief Icon-mode view of the card database, one card picture per item.
 */

#ifndef CARD_DATABASE_VISUAL_VIEW_H
#define CARD_DATABASE_VISUAL_VIEW_H

#include <QListView>
#include <QSize>
#include <qtmetamacros.h>

class CardDatabaseVisualDisplayModel;
class ExactCard;
class QEvent;
class QMouseEvent;
class QPoint;

/**
 * @brief A wrapping grid of card pictures.
 *
 * The view virtualizes painting: it only asks the delegate for the items that are
 * on screen, so the whole card database can sit in the model without a widget (or
 * pixmap) per card. Clicks and hover are reported with the exact card under the
 * cursor, which is what the deck editor needs to add, decrement or preview a card.
 */
class CardDatabaseVisualView : public QListView
{
    Q_OBJECT

public:
    explicit CardDatabaseVisualView(CardDatabaseVisualDisplayModel *_model, QWidget *parent = nullptr);

    /**
     * @brief Sets the card size as a percentage of the base card width.
     * @param factor Percentage in the range of the card size slider.
     */
    void setCardSizeFactor(int factor);

signals:
    /** @brief A card was pressed. The event is valid for the duration of the emission. */
    void cardClicked(QMouseEvent *event, const ExactCard &card);
    /** @brief The cursor moved onto a different card. */
    void cardHovered(const ExactCard &card);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool viewportEvent(QEvent *event) override;

private:
    void updateHoveredRow(const QPoint &position);

    CardDatabaseVisualDisplayModel *model;
    int cardSizeFactor = 100;
    int hoveredRow = -1;
};

#endif // CARD_DATABASE_VISUAL_VIEW_H
