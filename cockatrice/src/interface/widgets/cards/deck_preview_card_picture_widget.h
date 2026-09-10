/**
 * @file deck_preview_card_picture_widget.h
 * @ingroup CardWidgets
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef DECK_PREVIEW_CARD_PICTURE_WIDGET_H
#define DECK_PREVIEW_CARD_PICTURE_WIDGET_H

#include "card_info_picture_with_text_overlay_widget.h"

#include <QColor>
#include <QSize>
#include <QTextOption>
#include <QTimer>

class DeckPreviewCardPictureWidget final : public CardInfoPictureWithTextOverlayWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a DeckPreviewCardPictureWidget.
     * @param parent The parent widget.
     * @param hoverToZoomEnabled If this widget will spawn a larger widget when hovered over.
     * @param raiseOnEnter If the widget raises its border when the mouse enters.
     * @param textColor The color of the overlay text.
     * @param outlineColor The color of the outline around the text.
     * @param fontSize The font size of the overlay text.
     * @param alignment The alignment of the text within the overlay.
     * @param emitClickImmediately If true, a left click is reported immediately on click
     *        instead of after the double-click interval. Use this for selection surfaces
     *        where reacting to a double-click (select-and-open) would needlessly delay the
     *        single-click feedback. The double-click signal is still emitted.
     */
    explicit DeckPreviewCardPictureWidget(QWidget *parent,
                                          bool hoverToZoomEnabled = false,
                                          bool raiseOnEnter = false,
                                          const QColor &textColor = Qt::white,
                                          const QColor &outlineColor = Qt::black,
                                          int fontSize = 12,
                                          Qt::Alignment alignment = Qt::AlignCenter,
                                          bool _emitClickImmediately = false);

signals:
    void imageClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageSingleClicked();
    void imageDoubleClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

private:
    QTimer *singleClickTimer;
    QMouseEvent *lastMouseEvent = nullptr; // Store the last mouse event
    bool emitClickImmediately;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // DECK_PREVIEW_CARD_PICTURE_WIDGET_H
