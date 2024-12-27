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

    // Create the grid to hold the checkboxes
    gridLayout = new QGridLayout(this);

    // Create the checkbox for sorting options visibility
    sortCheckBox = new QCheckBox(this);
    sortCheckBox->setText(tr("Display Sorting Options"));
    sortCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSortOptionsVisible());
    connect(sortCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilitySortOptions);
    connect(sortCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSortOptionsVisible);

    // Create the checkbox for search bar visibility
    searchCheckBox = new QCheckBox(this);
    searchCheckBox->setText(tr("Display Search Bar"));
    searchCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorSearchBarVisible());
    connect(searchCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilitySearchBar);
    connect(searchCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorSearchBarVisible);

    // Create the checkbox for card size slider visibility
    cardSizeCheckBox = new QCheckBox(this);
    cardSizeCheckBox->setText(tr("Display Card Size Slider"));
    cardSizeCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorCardSizeSliderVisible());
    connect(cardSizeCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilityCardSizeSlider);
    connect(cardSizeCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorCardSizeSliderVisible);

    // Create the checkbox for navigation buttons visibility
    navigationCheckBox = new QCheckBox(this);
    navigationCheckBox->setText(tr("Display Navigation Buttons"));
    navigationCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorNavigationButtonsVisible());
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, printingSelector,
            &PrintingSelector::toggleVisibilityNavigationButtons);
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorNavigationButtonsVisible);

    // Add checkboxes to the grid
    gridLayout->addWidget(sortCheckBox, 0, 0);
    gridLayout->addWidget(searchCheckBox, 0, 1);
    gridLayout->addWidget(cardSizeCheckBox, 1, 0);
    gridLayout->addWidget(navigationCheckBox, 1, 1);

    // Add grid to the main layout
    layout->addLayout(gridLayout);
}
