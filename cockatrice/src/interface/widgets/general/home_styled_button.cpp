#include "home_styled_button.h"

#include <QPainter>
#include <QPainterPath>
#include <qgraphicseffect.h>
#include <qstyleoption.h>

HomeStyledButton::HomeStyledButton(const QString &text, QPair<QColor, QColor> _gradientColors, QWidget *parent)
    : QPushButton(text, parent), gradientColors(_gradientColors)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(25);
    setStyleSheet(generateButtonStylesheet(gradientColors));
}

void HomeStyledButton::updateStylesheet(const QPair<QColor, QColor> &colors)
{
    gradientColors = colors;
    setStyleSheet(generateButtonStylesheet(gradientColors));
}

QString HomeStyledButton::generateButtonStylesheet(const QPair<QColor, QColor> &colors)
{
    QColor baseGradientStart = colors.first;
    QColor baseGradientEnd = colors.second;

    QColor hoverGradientStart = baseGradientStart.lighter(120); // 20% lighter
    QColor hoverGradientEnd = baseGradientEnd.lighter(120);

    QColor pressedGradientStart = baseGradientStart.darker(130); // 30% darker
    QColor pressedGradientEnd = baseGradientEnd.darker(130);

    // Disabled: more gray, less saturated
    QColor disabledGradientStart = baseGradientStart.toHsv();
    disabledGradientStart.setHsv(disabledGradientStart.hue(), disabledGradientStart.saturation() * 0.2,
                                 disabledGradientStart.value() * 0.6);
    QColor disabledGradientEnd = baseGradientEnd.toHsv();
    disabledGradientEnd.setHsv(disabledGradientEnd.hue(), disabledGradientEnd.saturation() * 0.2,
                               disabledGradientEnd.value() * 0.6);

    return QString(R"(
        QPushButton {
            font-size: 34px;
            padding: 30px;
            color: white;
            border: 2px solid %1;
            border-radius: 20px;
            background: qlineargradient(x1:0, y1:1, x2:0, y2:0,
                                        stop:0 %2, stop:1 %3);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:1, x2:0, y2:0,
                                        stop:0 %4, stop:1 %5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 %6, stop:1 %7);
        }
        QPushButton:disabled {
            color: #aaaaaa;
            border: 2px solid #888888;
            background: qlineargradient(x1:0, y1:1, x2:0, y2:0,
                                        stop:0 %8, stop:1 %9);
        }
    )")
        .arg(baseGradientStart.name())
        .arg(baseGradientStart.name())
        .arg(baseGradientEnd.name())
        .arg(hoverGradientStart.name())
        .arg(hoverGradientEnd.name())
        .arg(pressedGradientStart.name())
        .arg(pressedGradientEnd.name())
        .arg(disabledGradientStart.name())
        .arg(disabledGradientEnd.name());
}

void HomeStyledButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); // Event is just used for update clipping, we redraw the whole widget.
    QStyleOptionButton opt;
    initStyleOption(&opt);
    opt.text.clear(); // prevent style from drawing text

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);

    // Draw white text with a black outline
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font = this->font();
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    QSize textSize = fm.size(Qt::TextSingleLine, this->text());
    QPointF center((width() - textSize.width()) / 2.0, (height() + textSize.height() / 2.0) / 2.0);

    QPainterPath path;
    path.addText(center, font, this->text());

    painter.setPen(QPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::white);
    painter.drawPath(path);
}
