#ifndef COCKATRICE_COLOR_BUTTON_H
#define COCKATRICE_COLOR_BUTTON_H

#include <QColor>
#include <QToolButton>

class ColorButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = nullptr);

    QColor getColor() const
    {
        return color;
    }
    void setColor(const QColor &c);

signals:
    void colorChanged(const QColor &color);

private slots:
    void pickColor();

private:
    void updateSwatch();
    QColor color;
};

#endif // COCKATRICE_COLOR_BUTTON_H