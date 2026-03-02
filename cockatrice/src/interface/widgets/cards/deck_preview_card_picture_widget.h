/**
 * @file deck_preview_card_picture_widget.h
 * @ingroup CardWidgets
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

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
    explicit DeckPreviewCardPictureWidget(QWidget *parent,
                                          bool hoverToZoomEnabled = false,
                                          bool raiseOnEnter = false,
                                          const QColor &textColor = Qt::white,
                                          const QColor &outlineColor = Qt::black,
                                          int fontSize = 12,
                                          Qt::Alignment alignment = Qt::AlignCenter);

signals:
    void imageClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

private:
    QTimer *singleClickTimer;
    QMouseEvent *lastMouseEvent = nullptr; // Store the last mouse event

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // DECK_PREVIEW_CARD_PICTURE_WIDGET_H
