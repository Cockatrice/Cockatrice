#ifndef DYNAMICFONTSIZEPUSHBUTTON_H
#define DYNAMICFONTSIZEPUSHBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>

class DynamicFontSizePushButton : public QPushButton
{
public:
    explicit DynamicFontSizePushButton(QWidget* parent = NULL);

    /* This method overwrite stylesheet */
    void setTextColor(QColor color);
    QColor getTextColor();

    void setTextAndColor(const QString &text, QColor color=QColor::Invalid);

    // QWidget interface
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor textColor;

};

#endif // DYNAMICFONTSIZEPUSHBUTTON_H