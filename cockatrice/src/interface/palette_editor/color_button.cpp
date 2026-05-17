#include "color_button.h"

#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget *parent) : QToolButton(parent)
{
    setFixedSize(52, 24);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Click to pick a color"));
    connect(this, &QToolButton::clicked, this, &ColorButton::pickColor);
}

void ColorButton::setColor(const QColor &c)
{
    if (color == c) {
        return;
    }
    color = c;
    updateSwatch();
    emit colorChanged(c);
}

void ColorButton::pickColor()
{
    QColor chosen = QColorDialog::getColor(color, this, tr("Pick colour"), QColorDialog::ShowAlphaChannel);
    if (chosen.isValid()) {
        setColor(chosen);
    }
}

void ColorButton::updateSwatch()
{
    QPixmap pixmap(size() * devicePixelRatioF());
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Checkerboard for alpha
    const int cellSize = 4;
    for (int y = 0; y < height(); y += cellSize) {
        for (int x = 0; x < width(); x += cellSize) {
            painter.fillRect(x, y, cellSize, cellSize,
                             ((x / cellSize + y / cellSize) % 2) ? QColor(180, 180, 180) : Qt::white);
        }
    }

    // Color fill
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);

    // Border
    QColor border = palette().color(QPalette::Shadow);
    border.setAlpha(180);
    painter.setPen(QPen(border, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);

    // Hex label — black or white for contrast
    int luma = 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();
    painter.setPen((luma > 128 && color.alpha() > 80) ? QColor(0, 0, 0, 180) : QColor(255, 255, 255, 200));
    QFont f = font();
    f.setPixelSize(8);
    painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, color.name().toUpper());

    setIcon(QIcon(pixmap));
    setIconSize(size());
    setText({});
}