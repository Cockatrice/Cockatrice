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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QMap<QString, int> colors;
    bool isHovered = false;
    const double minRatioThreshold = 0.01; // skip tiny slices

    QColor colorFromName(const QString &name) const;
    QString tooltipForPoint(const QPoint &pt) const;
};

#endif // COLOR_PIE_H

#endif // COCKATRICE_COLOR_PIE_H
