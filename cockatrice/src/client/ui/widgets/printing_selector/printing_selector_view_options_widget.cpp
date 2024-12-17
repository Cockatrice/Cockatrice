#include "printing_selector_view_options_widget.h"

#include "../../../../settings/cache_settings.h"

/**
 * @class PrintingSelectorViewOptionsWidget
 * @brief A widget that provides the view options for the PrintingSelector, including checkboxes
 * for sorting, search bar, card size slider, and navigation buttons.
 *
 * This widget allows the user to toggle the visibility of various interface components of the
 * PrintingSelector through checkboxes. The state of the checkboxes is saved and restored using
 * the `SettingsCache`.
 */
PrintingSelectorViewOptionsWidget::PrintingSelectorViewOptionsWidget(QWidget *parent,
                                                                     PrintingSelector *_printingSelector)
    : QWidget(parent), printingSelector(_printingSelector)
{
    // Set up the layout for the widget
    layout = new QHBoxLayout(this);
    setLayout(layout);

    // Create the flow widget to hold the checkboxes
    flowWidget = new FlowWidget(this, Qt::ScrollBarPolicy::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    // Create the checkbox for sorting options visibility
    sortCheckBox = new QCheckBox(flowWidget);
    sortCheckBox->setText(tr("Display Sorting Options"));
    sortCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSortOptionsVisible());
    connect(sortCheckBox, &QCheckBox::stateChanged, printingSelector, &PrintingSelector::toggleVisibilitySortOptions);
    connect(sortCheckBox, &QCheckBox::stateChanged, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSortOptionsVisible);

    // Create the checkbox for search bar visibility
    searchCheckBox = new QCheckBox(flowWidget);
    searchCheckBox->setText(tr("Display Search Bar"));
    searchCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSearchBarVisible());
    connect(searchCheckBox, &QCheckBox::stateChanged, printingSelector, &PrintingSelector::toggleVisibilitySearchBar);
    connect(searchCheckBox, &QCheckBox::stateChanged, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSearchBarVisible);

    // Create the checkbox for card size slider visibility
    cardSizeCheckBox = new QCheckBox(flowWidget);
    cardSizeCheckBox->setText(tr("Display Card Size Slider"));
    cardSizeCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorCardSizeSliderVisible());
    connect(cardSizeCheckBox, &QCheckBox::stateChanged, printingSelector,
            &PrintingSelector::toggleVisibilityCardSizeSlider);
    connect(cardSizeCheckBox, &QCheckBox::stateChanged, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorCardSizeSliderVisible);

    // Create the checkbox for navigation buttons visibility
    navigationCheckBox = new QCheckBox(flowWidget);
    navigationCheckBox->setText(tr("Display Navigation Buttons"));
    navigationCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorNavigationButtonsVisible());
    connect(navigationCheckBox, &QCheckBox::stateChanged, printingSelector,
            &PrintingSelector::toggleVisibilityNavigationButtons);
    connect(navigationCheckBox, &QCheckBox::stateChanged, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorNavigationButtonsVisible);

    // Add checkboxes to the flow widget
    flowWidget->addWidget(sortCheckBox);
    flowWidget->addWidget(searchCheckBox);
    flowWidget->addWidget(cardSizeCheckBox);
    flowWidget->addWidget(navigationCheckBox);

    // Add flow widget to the main layout
    layout->addWidget(flowWidget);
}
