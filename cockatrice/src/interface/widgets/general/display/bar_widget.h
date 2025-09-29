/**
 * @file bar_widget.h
 * @ingroup Widgets
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef BAR_WIDGET_H
#define BAR_WIDGET_H

#include <QColor>
#include <QString>
#include <QWidget>

class BarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarWidget(QString label, int value, int total, QColor barColor = Qt::blue, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString label;
    int value;
    int total;
    QColor barColor; // Store the bar color
};

#endif // BAR_WIDGET_H
