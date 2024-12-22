#include "deck_preview_card_picture_widget.h"

#include <QFontMetrics>
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
}

void DeckPreviewCardPictureWidget::mousePressEvent(QMouseEvent *event)
{
    emit imageClicked(event, this);
}

void DeckPreviewCardPictureWidget::setFilePath(const QString &_filePath)
{
    filePath = _filePath;
}
