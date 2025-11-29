#ifndef COCKATRICE_COLOR_BAR_H
#define COCKATRICE_COLOR_BAR_H

#include <QColor>
#include <QMap>
#include <QString>
#include <QWidget>

class ColorBar : public QWidget
{
    Q_OBJECT

public:
    explicit ColorBar(const QMap<QString, int> &colors, QWidget *parent = nullptr);

    void setColors(const QMap<QString, int> &colors);
    void setMinPercentThreshold(double treshold)
    {
        minRatioThreshold = treshold / 100.0; // store as ratio
    }
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QMap<QString, int> colors;
    bool isHovered = false;
    double minRatioThreshold = 0.0; // 0.05 means 5%

    QColor colorFromName(const QString &name) const;
    QString tooltipForPosition(int x) const;
};

#endif // COCKATRICE_COLOR_BAR_H
