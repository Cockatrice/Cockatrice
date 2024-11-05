#ifndef CARD_PICTURE_WITH_TEXT_OVERLAY_H
#define CARD_PICTURE_WITH_TEXT_OVERLAY_H

#include "card_info_picture_widget.h"
#include <QColor>
#include <QSize>
#include <QTextOption>

class CardInfoPictureWithTextOverlayWidget : public CardInfoPictureWidget {
    Q_OBJECT

public:
    explicit CardInfoPictureWithTextOverlayWidget(QWidget *parent = nullptr,
                                        bool hoverToZoomEnabled = false,
                                        const QColor &textColor = Qt::white,
                                        const QColor &outlineColor = Qt::black,
                                        int fontSize = 12,
                                        Qt::Alignment alignment = Qt::AlignCenter);

    void setOverlayText(const QString &text);
    void setTextColor(const QColor &color);
    void setOutlineColor(const QColor &color);
    void setFontSize(int size);
    void setTextAlignment(Qt::Alignment alignment);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;

private:
    void drawOutlinedText(QPainter &painter, const QRect &textRect, const QString &text, const QTextOption &textOption);

    QString overlayText;
    QColor textColor;
    QColor outlineColor;
    int fontSize;
    Qt::Alignment textAlignment;
};

#endif // CARD_PICTURE_WITH_TEXT_OVERLAY_H
