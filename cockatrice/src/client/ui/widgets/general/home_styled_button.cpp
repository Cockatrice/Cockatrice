#include "home_styled_button.h"

#include <QPainter>
#include <QPainterPath>
#include <qgraphicseffect.h>

HomeStyledButton::HomeStyledButton(const QString &text, QPair<QColor, QColor> _gradientColors, QWidget *parent)
    : QPushButton(text, parent), gradientColors(_gradientColors)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(50);
    setStyleSheet(generateButtonStylesheet(gradientColors));
}

void HomeStyledButton::updateStylesheet(const QPair<QColor, QColor> &colors)
{
    gradientColors = colors;
    setStyleSheet(generateButtonStylesheet(gradientColors));
}

QString HomeStyledButton::generateButtonStylesheet(const QPair<QColor, QColor> &colors)
{
    QColor base1 = colors.first;
    QColor base2 = colors.second;

    QColor hover1 = base1.lighter(120); // 20% lighter
    QColor hover2 = base2.lighter(120);

    QColor pressed1 = base1.darker(130); // 30% darker
    QColor pressed2 = base2.darker(130);

    // Disabled: more gray, less saturated
    QColor disabled1 = base1.toHsv();
    disabled1.setHsv(disabled1.hue(), disabled1.saturation() * 0.2, disabled1.value() * 0.6);
    QColor disabled2 = base2.toHsv();
    disabled2.setHsv(disabled2.hue(), disabled2.saturation() * 0.2, disabled2.value() * 0.6);

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
        .arg(base1.name())      // border color
        .arg(base1.name())      // normal gradient start
        .arg(base2.name())      // normal gradient end
        .arg(hover1.name())     // hover start
        .arg(hover2.name())     // hover end
        .arg(pressed1.name())   // pressed start
        .arg(pressed2.name())   // pressed end
        .arg(disabled1.name())  // disabled start
        .arg(disabled2.name()); // disabled end
}

void HomeStyledButton::paintEvent(QPaintEvent *event)
{
    QString originalText = text();
    setText(""); // Prevent QPushButton from drawing the text

    QPushButton::paintEvent(event); // Draw background, borders, etc.

    setText(originalText); // Restore text for internal logic

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font = this->font();
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    QSize textSize = fm.size(Qt::TextSingleLine, originalText);
    QPointF center((width() - textSize.width()) / 2.0, (height() + textSize.height() / 2.0) / 2.0);

    QPainterPath path;
    path.addText(center, font, originalText);

    painter.setPen(QPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::white);
    painter.drawPath(path);
}
