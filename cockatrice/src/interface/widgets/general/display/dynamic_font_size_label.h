/**
 * @file dynamic_font_size_label.h
 * @ingroup Widgets
 * @brief TODO: Document this.
 */

#ifndef DYNAMICFONTSIZELABEL_H
#define DYNAMICFONTSIZELABEL_H

#include <QColor>
#include <QLabel>

class DynamicFontSizeLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DynamicFontSizeLabel(QWidget *parent = NULL, Qt::WindowFlags f = Qt::WindowFlags());

    ~DynamicFontSizeLabel()
    {
    }

    static float getWidgetMaximumFontSize(QWidget *widget, const QString &text);

    /* This method overwrite stylesheet */
    void setTextColor(QColor color);
    QColor getTextColor();
    void setTextAndColor(const QString &text, QColor color = QColor::Invalid);
signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    QColor textColor;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);

    // QWidget interface
public:
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

#endif // DYNAMICFONTSIZELABEL_H