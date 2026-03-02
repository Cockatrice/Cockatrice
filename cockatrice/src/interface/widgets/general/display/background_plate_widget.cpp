#include "background_plate_widget.h"

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPen>

BackgroundPlateWidget::BackgroundPlateWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true); // For automatic background filling
}

void BackgroundPlateWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Set the background color to semi-transparent black with rounded corners
    QRect rect = this->rect();
    painter.setPen(Qt::NoPen); // No border
    if (focused) {
        painter.setBrush(QColor(85, 190, 75, 140));
    } else {
        painter.setBrush(QColor(0, 0, 0, 140)); // semi-transparent black
    }
    painter.drawRoundedRect(rect, 6, 6); // rounded corners
}

void BackgroundPlateWidget::setFocused(bool _focused)
{
    focused = _focused;
    update();
}