#ifndef PERCENT_BAR_WIDGET_H
#define PERCENT_BAR_WIDGET_H

#include <QColor>
#include <QPainter>
#include <QWidget>

class PercentBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PercentBarWidget(QWidget *parent, double initialValue);

    void setValue(double newValue)
    {
        valueToDisplay = qBound(-100.0, newValue, 100.0); // Clamp to [-100, 100]
        update();                                         // Trigger repaint
    }

    double value() const
    {
        return valueToDisplay;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double valueToDisplay; // Ranges from -100 to 100
};

#endif // PERCENT_BAR_WIDGET_H
