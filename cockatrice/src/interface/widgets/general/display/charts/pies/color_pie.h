#ifndef COCKATRICE_COLOR_PIE_H
#define COCKATRICE_COLOR_PIE_H

#ifndef COLOR_PIE_H
#define COLOR_PIE_H

#include <QMap>
#include <QString>
#include <QWidget>

class ColorPie : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPie(const QMap<QString, int> &_colors = {}, QWidget *parent = nullptr);

    void setColors(const QMap<QString, int> &_colors);

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QList<QPair<QString, int>> colors;
    bool isHovered = false;
    const double minRatioThreshold = 0.01; // skip tiny slices

    QColor colorFromName(const QString &name) const;
    QString tooltipForPoint(const QPoint &pt) const;
};

#endif // COLOR_PIE_H

#endif // COCKATRICE_COLOR_PIE_H
