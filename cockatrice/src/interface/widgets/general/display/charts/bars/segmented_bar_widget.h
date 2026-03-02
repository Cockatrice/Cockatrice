#ifndef COCKATRICE_SEGMENTED_BAR_WIDGET_H
#define COCKATRICE_SEGMENTED_BAR_WIDGET_H

#include <QColor>
#include <QVector>
#include <QWidget>

class SegmentedBarWidget : public QWidget
{
    Q_OBJECT

public:
    struct Segment
    {
        QString category;
        int value = 0;
        QStringList cards;
        QColor color;
    };

    QString label;
    QVector<Segment> segments;
    float total = 1.0;

    explicit SegmentedBarWidget(QString label, QVector<Segment> segments, int total, QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    int segmentAt(int y) const;

private:
    bool hovered = true;
};

#endif // COCKATRICE_SEGMENTED_BAR_WIDGET_H
