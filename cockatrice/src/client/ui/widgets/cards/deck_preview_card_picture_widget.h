#ifndef DECK_PREVIEW_CARD_PICTURE_WIDGET_H
#define DECK_PREVIEW_CARD_PICTURE_WIDGET_H

#include "card_info_picture_with_text_overlay_widget.h"

#include <QColor>
#include <QSize>
#include <QTextOption>

class DeckPreviewCardPictureWidget final : public CardInfoPictureWithTextOverlayWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewCardPictureWidget(QWidget *parent = nullptr,
                                          bool hoverToZoomEnabled = false,
                                          const QColor &textColor = Qt::white,
                                          const QColor &outlineColor = Qt::black,
                                          int fontSize = 12,
                                          Qt::Alignment alignment = Qt::AlignCenter);

    QString filePath;

signals:
    void imageClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

public slots:
    void setFilePath(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // DECK_PREVIEW_CARD_PICTURE_WIDGET_H
