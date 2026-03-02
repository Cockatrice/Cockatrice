#ifndef COCKATRICE_BAR_CHART_BACKGROUND_WIDGET_H
#define COCKATRICE_BAR_CHART_BACKGROUND_WIDGET_H

#include <QPainter>
#include <QWidget>

class BarChartBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    int highest = 0;      // global maximum (shared across bars)
    int barCount = 0;     // number of CMC columns
    int labelHeight = 20; // reserved for CMC numbers

    explicit BarChartBackgroundWidget(QWidget *parent);
public slots:
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // COCKATRICE_BAR_CHART_BACKGROUND_WIDGET_H
