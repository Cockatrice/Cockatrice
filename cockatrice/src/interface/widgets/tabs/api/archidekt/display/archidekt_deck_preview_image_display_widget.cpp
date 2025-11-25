#include "archidekt_deck_preview_image_display_widget.h"

#include <QFontMetrics>
#include <QPainter>

ArchidektDeckPreviewImageDisplayWidget::ArchidektDeckPreviewImageDisplayWidget(QWidget *parent) : QWidget(parent)
{
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);

    topLeftLabel = new OverlayLabel(this);
    topRightLabel = new OverlayLabel(this);
    bottomLeftLabel = new OverlayLabel(this);
    bottomRightLabel = new OverlayLabel(this);

    QFont f;
    f.setBold(true);
    topLeftLabel->setFont(f);
    topRightLabel->setFont(f);
    bottomLeftLabel->setFont(f);
    bottomRightLabel->setFont(f);

    // Raise so labels appear above image
    topLeftLabel->raise();
    topRightLabel->raise();
    bottomLeftLabel->raise();
    bottomRightLabel->raise();
}

void ArchidektDeckPreviewImageDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Full size for the image
    imageLabel->setGeometry(rect());

    topLeftLabel->adjustSize();
    topRightLabel->adjustSize();
    bottomLeftLabel->adjustSize();
    bottomRightLabel->adjustSize();

    // Padding settings
    const int horizontalPadding = 8;
    const int topPadding = 6;
    const int bottomPadding = 6;

    // Left-aligned, top-aligned (Deck Name)
    topLeftLabel->move(horizontalPadding, topPadding);

    // Right-aligned, top-aligned (Card Count)
    topRightLabel->move(width() - topRightLabel->width() - horizontalPadding, topPadding);

    // Bottom-left, bottom-aligned (EDH bracket)
    bottomLeftLabel->move(horizontalPadding, height() - bottomLeftLabel->height() - bottomPadding);

    // Bottom-right, bottom-aligned (views)
    bottomRightLabel->move(width() - bottomRightLabel->width() - horizontalPadding,
                           height() - bottomRightLabel->height() - bottomPadding);

    // Ensure labels stay above image
    topLeftLabel->raise();
    topRightLabel->raise();
    bottomLeftLabel->raise();
    bottomRightLabel->raise();
}
