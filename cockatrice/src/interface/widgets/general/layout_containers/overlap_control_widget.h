/**
 * @file overlap_control_widget.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef OVERLAP_CONTROL_WIDGET_H
#define OVERLAP_CONTROL_WIDGET_H
#include "../display/labeled_input.h"
#include "overlap_widget.h"

#include <QHBoxLayout>
#include <QSlider>
#include <QWidget>

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
