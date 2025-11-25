#include "overlay_label.h"

#include <QPainter>

OverlayLabel::OverlayLabel(QWidget *parent) : QLabel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void OverlayLabel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect r = rect().adjusted(0, 0, -1, -1);

    p.setBrush(QColor(0, 0, 0, 140));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(r, 6, 6);

    QLabel::paintEvent(event);
}
