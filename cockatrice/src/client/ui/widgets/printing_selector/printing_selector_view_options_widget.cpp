#include "printing_selector_view_options_widget.h"

#include "../../../../settings/cache_settings.h"

PrintingSelectorViewOptionsWidget::PrintingSelectorViewOptionsWidget(QWidget *parent,
                                                                     PrintingSelector *_printingSelector)
    : QWidget(parent), printingSelector(_printingSelector)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    flowWidget = new FlowWidget(this, Qt::ScrollBarPolicy::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    sortCheckBox = new QCheckBox(flowWidget);
    sortCheckBox->setText(tr("Display Sorting Options"));
    sortCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSortOptionsVisible());
    connect(sortCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilitySortOptions);
    connect(sortCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSortOptionsVisible);
    searchCheckBox = new QCheckBox(flowWidget);
    searchCheckBox->setText(tr("Display Search Bar"));
    searchCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSearchBarVisible());
    connect(searchCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilitySearchBar);
    connect(searchCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSearchBarVisible);
    cardSizeCheckBox = new QCheckBox(flowWidget);
    cardSizeCheckBox->setText(tr("Display Card Size Slider"));
    cardSizeCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorCardSizeSliderVisible());
    connect(cardSizeCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilityCardSizeSlider);
    connect(cardSizeCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorCardSizeSliderVisible);
    navigationCheckBox = new QCheckBox(flowWidget);
    navigationCheckBox->setText(tr("Display Navigation Buttons"));
    navigationCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorNavigationButtonsVisible());
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilityNavigationButtons);
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorNavigationButtonsVisible);

    flowWidget->addWidget(sortCheckBox);
    flowWidget->addWidget(searchCheckBox);
    flowWidget->addWidget(cardSizeCheckBox);
    flowWidget->addWidget(navigationCheckBox);

    layout->addWidget(flowWidget);
}