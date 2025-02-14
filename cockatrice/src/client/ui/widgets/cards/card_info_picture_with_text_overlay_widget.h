#ifndef CARD_PICTURE_WITH_TEXT_OVERLAY_H
#define CARD_PICTURE_WITH_TEXT_OVERLAY_H

#include "card_info_picture_widget.h"

#include <QColor>
#include <QSize>
#include <QTextOption>
#include <qpropertyanimation.h>

class CardInfoPictureWithTextOverlayWidget : public CardInfoPictureWidget
{
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

    [[nodiscard]] QSize sizeHint() const override;
signals:
    void imageClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    [[nodiscard]] QSize minimumSizeHint() const override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    void drawOutlinedText(QPainter &painter,
                          const QRect &textRect,
                          const QString &text,
                          const QTextOption &textOption) const;

    QString overlayText;
    QColor textColor;
    QColor outlineColor;
    int fontSize;
    Qt::Alignment textAlignment;
    QPropertyAnimation *animation;
    QPoint originalPos;         // Store the original position
    const int hoverOffset = 10; // Adjust this for how much the widget moves up
};

#endif // CARD_PICTURE_WITH_TEXT_OVERLAY_H
