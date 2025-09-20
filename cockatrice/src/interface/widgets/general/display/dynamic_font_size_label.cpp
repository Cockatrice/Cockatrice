#include "dynamic_font_size_label.h"
#define FONT_PRECISION (0.5)

#include <QDebug>
#include <QElapsedTimer>

DynamicFontSizeLabel::DynamicFontSizeLabel(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    setIndent(0);
}

void DynamicFontSizeLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit clicked();
}

void DynamicFontSizeLabel::paintEvent(QPaintEvent *event)
{
    // QElapsedTimer timer;
    // timer.start();

    QFont newFont = font();
    float fontSize = getWidgetMaximumFontSize(this, this->text());
    newFont.setPointSizeF(fontSize);
    setFont(newFont);
    // qDebug() << "Font size set to" << fontSize;

    QLabel::paintEvent(event);
    // LOG(true, "Paint delay" << ((float)timer.nsecsElapsed())/1000000.0 << " mS");
}

float DynamicFontSizeLabel::getWidgetMaximumFontSize(QWidget *widget, const QString &text)
{
    QFont font = widget->font();
    const QRect widgetRect = widget->contentsRect();
    const float widgetWidth = widgetRect.width();
    const float widgetHeight = widgetRect.height();

    QRectF newFontSizeRect;
    float currentSize = font.pointSizeF();

    float step = currentSize / 2.0;

    /* If too small, increase step */
    if (step <= FONT_PRECISION) {
        step = FONT_PRECISION * 4.0;
    }

    float lastTestedSize = currentSize;

    float currentHeight = 0;
    float currentWidth = 0;
    if (text == "") {
        return currentSize;
    }

    if (currentSize < 0) {
        return 1;
    }

    /* Only stop when step is small enough and new size is smaller than QWidget */
    while (step > FONT_PRECISION || (currentHeight > widgetHeight) || (currentWidth > widgetWidth)) {
        /* Keep last tested value */
        lastTestedSize = currentSize;

        /* Test label with its font */
        font.setPointSizeF(currentSize);
        /* Use font metrics to test */
        QFontMetricsF fm(font);

        /* Check if widget is QLabel */
        QLabel *label = qobject_cast<QLabel *>(widget);
        if (label) {
            newFontSizeRect =
                fm.boundingRect(widgetRect, (label->wordWrap() ? Qt::TextWordWrap : 0) | label->alignment(), text);
        } else {
            newFontSizeRect = fm.boundingRect(widgetRect, 0, text);
        }

        currentHeight = newFontSizeRect.height();
        currentWidth = newFontSizeRect.width();

        /* If new font size is too big, decrease it */
        if ((currentHeight > widgetHeight) || (currentWidth > widgetWidth)) {
            // qDebug() << "-- contentsRect()" << label->contentsRect() << "rect"<< label->rect() << " newFontSizeRect"
            // << newFontSizeRect << "Tight" << text << currentSize;
            currentSize -= step;
            /* if step is small enough, keep it constant, so it converge to biggest font size */
            if (step > FONT_PRECISION) {
                step /= 2.0;
            }
            /* Do not allow negative size */
            if (currentSize <= 0) {
                break;
            }
        }
        /* If new font size is smaller than maximum possible size, increase it */
        else {
            // qDebug() << "++ contentsRect()" << label->contentsRect() << "rect"<< label->rect() << " newFontSizeRect"
            // << newFontSizeRect << "Tight" << text << currentSize;
            currentSize += step;
        }
    }
    return lastTestedSize;
}

void DynamicFontSizeLabel::setTextColor(QColor color)
{
    if (color.isValid() && color != textColor) {
        textColor = color;
        setStyleSheet("color : " + color.name() + ";");
    }
}

QColor DynamicFontSizeLabel::getTextColor()
{
    return textColor;
}

void DynamicFontSizeLabel::setTextAndColor(const QString &text, QColor color)
{
    setTextColor(color);
    setText(text);
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizeLabel::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizeLabel::sizeHint() const
{
    return QWidget::sizeHint();
}