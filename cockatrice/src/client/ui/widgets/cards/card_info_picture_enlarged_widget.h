#ifndef CARD_PICTURE_ENLARGED_WIDGET_H
#define CARD_PICTURE_ENLARGED_WIDGET_H

#include "../../../../game/cards/card_database.h"

#include <QPixmap>
#include <QWidget>

class CardInfoPictureEnlargedWidget final : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit CardInfoPictureEnlargedWidget(QWidget *parent = nullptr);

    // Sets the card pixmap to display
    void setCardPixmap(CardInfoPtr card, QSize size);

protected:
    // Handles the painting event for the enlarged card
    void paintEvent(QPaintEvent *event) override;

private:
    // Cached pixmap for the enlarged card
    QPixmap enlargedPixmap;

    // Tracks if the pixmap needs to be refreshed/redrawn
    bool pixmapDirty;

    // Card information (card data pointer)
    CardInfoPtr info;

    // Loads the enlarged card pixmap
    void loadPixmap(const QSize &size);
};

#endif // CARD_PICTURE_ENLARGED_WIDGET_H
