//
// Created by Ascor on 25-Nov-25.
//

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

    QColor colorFromName(const QString &name) const;
    QString tooltipForPosition(int x) const;
};

#endif // COCKATRICE_COLOR_BAR_H
