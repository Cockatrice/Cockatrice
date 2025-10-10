/**
 * @file card_info_picture_enlarged_widget.h
 * @ingroup CardWigets
 * @ingroup DeckEditorWidgets
 * @brief TODO: Document this.
 */

#ifndef CARD_PICTURE_ENLARGED_WIDGET_H
#define CARD_PICTURE_ENLARGED_WIDGET_H

#include <QPixmap>
#include <QWidget>
#include <libcockatrice/card/printing/exact_card.h>

class CardInfoPictureEnlargedWidget final : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit CardInfoPictureEnlargedWidget(QWidget *parent = nullptr);

    // Sets the card pixmap to display
    void setCardPixmap(const ExactCard &_card, QSize size);

protected:
    // Handles the painting event for the enlarged card
    void paintEvent(QPaintEvent *event) override;

private:
    // Cached pixmap for the enlarged card
    QPixmap enlargedPixmap;

    // Tracks if the pixmap needs to be refreshed/redrawn
    bool pixmapDirty;

    // Card information
    ExactCard card;

    // Loads the enlarged card pixmap
    void loadPixmap(const QSize &size);
};

#endif // CARD_PICTURE_ENLARGED_WIDGET_H
