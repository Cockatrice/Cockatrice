#include "printing_selector_view_options_widget.h"

PrintingSelectorViewOptionsWidget::PrintingSelectorViewOptionsWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    flowWidget = new FlowWidget(this, Qt::ScrollBarPolicy::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    sortCheckBox = new QCheckBox(flowWidget);
    sortCheckBox->setText(tr("Display Sorting Options"));
    searchCheckBox = new QCheckBox(flowWidget);
    searchCheckBox->setText(tr("Display Search Bar"));
    cardSizeCheckBox = new QCheckBox(flowWidget);
    cardSizeCheckBox->setText(tr("Display Card Size Slider"));
    navigationCheckBox = new QCheckBox(flowWidget);
    navigationCheckBox->setText(tr("Display Navigation Buttons"));

    flowWidget->addWidget(sortCheckBox);
    flowWidget->addWidget(searchCheckBox);
    flowWidget->addWidget(cardSizeCheckBox);
    flowWidget->addWidget(navigationCheckBox);

    layout->addWidget(flowWidget);
}