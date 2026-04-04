/**
 * @file overlap_control_widget.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef OVERLAP_CONTROL_WIDGET_H
#define OVERLAP_CONTROL_WIDGET_H

#include <QWidget>

class OverlapWidget;
class QHBoxLayout;
class LabeledInput;
class QSlider;
class OverlapControlWidget final : public QWidget
{
    Q_OBJECT

public:
    OverlapControlWidget(int overlapPercentage,
                         int maxColumns,
                         int maxRows,
                         Qt::Orientation direction,
                         QWidget *parent);
    void connectOverlapWidget(OverlapWidget *overlap_widget);

private:
    QHBoxLayout *layout;
    QSlider *card_size_slider;
    LabeledInput *amount_of_items_to_overlap;
    LabeledInput *overlap_percentage_input;
    LabeledInput *overlap_direction;
    int overlapPercentage;
    int maxColumns;
    int maxRows;
    Qt::Orientation direction;
};

#endif // OVERLAP_CONTROL_WIDGET_H
