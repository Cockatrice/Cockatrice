#ifndef COCKATRICE_BAR_CHART_WIDGET_H
#define COCKATRICE_BAR_CHART_WIDGET_H

#include <QColor>
#include <QString>
#include <QVector>
#include <QWidget>

struct BarSegment
{
    QString category;
    int value;
    QStringList cards;
    QColor color;
};

struct BarData
{
    QString label;
    QVector<BarSegment> segments;
};

class BarChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget *parent = nullptr);

    void setBars(const QVector<BarData> &bars);
    void setHighest(int h); // global max for scaling
    int barCount() const
    {
        return bars.size();
    }

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QVector<BarData> bars;
    int highest = 1; // global maximum value

    int hoveredBar = -1;
    int hoveredSegment = -1;
};

#endif // COCKATRICE_BAR_CHART_WIDGET_H
