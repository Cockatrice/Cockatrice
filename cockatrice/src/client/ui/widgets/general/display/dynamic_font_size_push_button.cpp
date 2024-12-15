#include "dynamic_font_size_push_button.h"

#include <QDebug>
#include "dynamic_font_size_label.h"

DynamicFontSizePushButton::DynamicFontSizePushButton(QWidget* parent)
    :QPushButton(parent)
{

}

void DynamicFontSizePushButton::paintEvent(QPaintEvent *event)
{
    //QElapsedTimer timer;
    //timer.start();

    QFont newFont = font();
    float fontSize = DynamicFontSizeLabel::getWidgetMaximumFontSize(this, this->text());
    newFont.setPointSizeF(fontSize);
    setFont(newFont);

    QPushButton::paintEvent(event);
    //LOG(true, "Paint delay" << ((float)timer.nsecsElapsed())/1000000.0 << " mS");
}

void DynamicFontSizePushButton::setTextColor(QColor color){
    if (color.isValid() && color!=textColor){
        textColor = color;
        setStyleSheet("color : "+color.name()+";");
    }
}

void DynamicFontSizePushButton::setTextAndColor(const QString &text, QColor color){
    setTextColor(color);
    setText(text);
}

QColor DynamicFontSizePushButton::getTextColor(){
    return textColor;
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::sizeHint() const
{
    return QWidget::sizeHint();
}