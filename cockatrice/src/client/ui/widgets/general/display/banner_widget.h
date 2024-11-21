#ifndef BANNER_WIDGET_H
#define BANNER_WIDGET_H

#include <QWidget>
#include <QLabel>

class BannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerWidget(const QString& text, Qt::Orientation orientation = Qt::Vertical, int transparency = 80, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* bannerLabel;
    Qt::Orientation gradientOrientation;
    int transparency; // Transparency percentage for the gradient
};

#endif //BANNER_WIDGET_H
