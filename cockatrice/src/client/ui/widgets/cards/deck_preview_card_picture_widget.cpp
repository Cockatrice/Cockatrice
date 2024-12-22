#include "deck_preview_card_picture_widget.h"

#include <QApplication>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStylePainter>
#include <QTextOption>

/**
 * @brief Constructs a CardPictureWithTextOverlay widget.
 * @param parent The parent widget.
 * @param hoverToZoomEnabled If this widget will spawn a larger widget when hovered over.
 * @param textColor The color of the overlay text.
 * @param outlineColor The color of the outline around the text.
 * @param fontSize The font size of the overlay text.
 * @param alignment The alignment of the text within the overlay.
 *
 * Sets the widget's size policy and default border style.
 */
DeckPreviewCardPictureWidget::DeckPreviewCardPictureWidget(QWidget *parent,
                                                           const bool hoverToZoomEnabled,
                                                           const QColor &textColor,
                                                           const QColor &outlineColor,
                                                           const int fontSize,
                                                           const Qt::Alignment alignment)
    : CardInfoPictureWithTextOverlayWidget(parent, hoverToZoomEnabled, textColor, outlineColor, fontSize, alignment)
{
    singleClickTimer = new QTimer(this);
    singleClickTimer->setSingleShot(true);
    connect(singleClickTimer, &QTimer::timeout, this, [this]() { qDebug() << "Clicked some stuff"; });
}

void DeckPreviewCardPictureWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastMouseEvent = event;
        singleClickTimer->start(QApplication::doubleClickInterval());
    }
}

void DeckPreviewCardPictureWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        singleClickTimer->stop(); // Prevent single-click logic
        emit imageClicked(lastMouseEvent, this);
    }
}

void DeckPreviewCardPictureWidget::setFilePath(const QString &_filePath)
{
    filePath = _filePath;
}
