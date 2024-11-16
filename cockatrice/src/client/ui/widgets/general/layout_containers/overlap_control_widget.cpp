#include "overlap_control_widget.h"

#include "overlap_widget.h"

OverlapControlWidget::OverlapControlWidget(int overlapPercentage,
                                           int maxColumns,
                                           int maxRows,
                                           Qt::Orientation direction,
                                           QWidget *parent)
    : QWidget(parent), overlapPercentage(overlapPercentage), maxColumns(maxColumns), maxRows(maxRows),
      direction(direction)
{
    // Main Widget and Layout
    this->setMinimumSize(0, 100);
    // this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // this->setStyleSheet("border: 10px solid red;");

    layout = new QHBoxLayout(this);
    this->setLayout(layout);

    card_size_slider = new QSlider(Qt::Horizontal);
    card_size_slider->setRange(1, 10); // Example range for scaling, adjust as needed

    amount_of_items_to_overlap = new LabeledInput(this, tr("Cards to overlap:"));
    amount_of_items_to_overlap->addSpinBox(0, 999, 10);
    overlap_percentage_input = new LabeledInput(this, tr("Overlap percentage:"));
    overlap_percentage_input->addSpinBox(0, 100, 80);
    overlap_direction = new LabeledInput(this, tr("Overlap direction:"));
    overlap_direction->addDirectionComboBox();

    layout->addWidget(card_size_slider);
    layout->addWidget(amount_of_items_to_overlap);
    layout->addWidget(overlap_percentage_input);
    layout->addWidget(overlap_direction);

    // TODO probably connect this to the parent
    // connect(card_size_slider, &QSlider::valueChanged, display, &CardPicture::setScaleFactor);
}

void OverlapControlWidget::connectOverlapWidget(OverlapWidget *overlap_widget)
{
    connect(amount_of_items_to_overlap, &LabeledInput::spinBoxValueChanged, overlap_widget,
            &OverlapWidget::maxOverlapItemsChanged);
    connect(overlap_direction, &LabeledInput::directionComboBoxChanged, overlap_widget,
            &OverlapWidget::overlapDirectionChanged);
}
