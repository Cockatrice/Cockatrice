#include "playmat_preview_widget.h"

#include "../cards/art_crop_attribution.h"
#include "playmat_utils.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

PlaymatPreviewWidget::PlaymatPreviewWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(400, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void PlaymatPreviewWidget::setPixmap(const QPixmap &pixmap)
{
    sourcePixmap = pixmap;
    update();
}

void PlaymatPreviewWidget::setParams(const PlaymatParams &p)
{
    params = p;
    update();
}

void PlaymatPreviewWidget::setAttribution(const QString &attribution)
{
    attributionText = attribution;
    update();
}

void PlaymatPreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = this->rect();
    const QColor accentColor(100, 116, 139);

    // Background
    const QRectF cardRect = QRectF(rect).adjusted(3, 2, -3, -2);
    QLinearGradient bg(cardRect.topLeft(), cardRect.topRight());
    bg.setColorAt(0, accentColor.darker(320));
    bg.setColorAt(1, QColor(18, 22, 30));
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRoundedRect(cardRect, 6, 6);
    painter.setBrush(accentColor);
    painter.drawRoundedRect(QRectF(cardRect.left(), cardRect.top(), 3, cardRect.height()), 2, 2);

    if (sourcePixmap.isNull()) {
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(rect, Qt::AlignCenter, tr("No card selected"));
        return;
    }

    // Draw the playmat art using the same logic as PlayerGraphicsItem
    // The preview area represents the combined stack+table play area
    // Stack is ~20% width on the left, table is ~80% on the right
    const QRectF playArea = cardRect.adjusted(6, 4, -4, -4);

    const QRectF srcRect = computeArtSourceRect(sourcePixmap.size(), params);
    const QRectF dstRect = coverFitRect(playArea, srcRect.size());

    painter.setClipRect(playArea.toRect());
    painter.drawPixmap(dstRect, sourcePixmap, srcRect);
    painter.setClipping(false);

    // Draw zone divider: stack is roughly the left portion
    const double stackWidthRatio = 0.18; // Stack is about 18% of total play area
    const double stackDividerX = playArea.left() + playArea.width() * stackWidthRatio;

    // Subtle semi-transparent overlays to distinguish zones
    // Stack zone overlay (slightly darker)
    QRectF stackOverlay(playArea.left(), playArea.top(), playArea.width() * stackWidthRatio, playArea.height());
    painter.fillRect(stackOverlay, QColor(0, 0, 0, 40));

    // Table zone overlay (very subtle)
    QRectF tableOverlay(stackDividerX, playArea.top(), playArea.width() * (1.0 - stackWidthRatio), playArea.height());
    painter.fillRect(tableOverlay, QColor(0, 0, 0, 20));

    // Zone divider line
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
    painter.drawLine(QPointF(stackDividerX, playArea.top()), QPointF(stackDividerX, playArea.bottom()));

    // Land divider line (about 60% down the table area)
    const double landDividerY = playArea.top() + playArea.height() * 0.65;
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter.drawLine(QPointF(stackDividerX, landDividerY), QPointF(playArea.right(), landDividerY));

    // Border around entire play area
    painter.setPen(QPen(QColor(70, 80, 95, 120), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(playArea.adjusted(0, 0, -1, -1), 3, 3);

    paintArtAttribution(painter, playArea, attributionText, Qt::AlignRight | Qt::AlignBottom, 0.8);
}
