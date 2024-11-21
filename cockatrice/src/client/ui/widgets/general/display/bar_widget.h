#ifndef BAR_WIDGET_H
#define BAR_WIDGET_H

#include <QPainter>
#include <QWidget>

class BarWidget : public QWidget
{
    Q_OBJECT

public:
    BarWidget(QString label, int value, int total, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    QString label;       // The CMC value (label for the bar)
    int value;           // The specific value for this CMC (the inner bar length)
    int total;           // Total value for calculating proportional bar length
    static int totalSum; // The total sum of all entry.second values (shared across instances)
};

#endif // BAR_WIDGET_H
