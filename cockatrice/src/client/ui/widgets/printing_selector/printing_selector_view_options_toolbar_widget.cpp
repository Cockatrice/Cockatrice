#include "printing_selector_view_options_toolbar_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @class PrintingSelectorViewOptionsToolbarWidget
 * @brief A widget that provides a toolbar for view options with collapsible and expandable functionality.
 *
 * This widget allows the user to collapse or expand the view options for the PrintingSelector,
 * providing a more compact interface when collapsed and a full view of options when expanded.
 */
PrintingSelectorViewOptionsToolbarWidget::PrintingSelectorViewOptionsToolbarWidget(QWidget *_parent,
                                                                                   PrintingSelector *_printingSelector)
    : QWidget(_parent), printingSelector(_printingSelector)
{
    // Set up layout for the widget
    layout = new QVBoxLayout();
    layout->setContentsMargins(9, 0, 9, 0);
    layout->setSpacing(0);
    setLayout(layout);

    // Set up the expanded widget with its layout
    expandedWidget = new QWidget(this);
    auto *expandedLayout = new QVBoxLayout(expandedWidget);
    expandedLayout->setContentsMargins(0, 0, 0, 0);
    expandedLayout->setSpacing(0);

    // Collapse button to toggle between expanded and collapsed states
    collapseButton = new QPushButton("▼", this);
    collapseButton->setFixedSize(20, 20);
    collapseButton->setToolTip("Collapse");
    collapseButton->setStyleSheet("border: none;");
    connect(collapseButton, &QPushButton::clicked, this, &PrintingSelectorViewOptionsToolbarWidget::collapse);
    expandedLayout->addWidget(collapseButton, 0, Qt::AlignLeft);

    // View options widget
    viewOptions = new PrintingSelectorViewOptionsWidget(expandedWidget, printingSelector);
    expandedLayout->addWidget(viewOptions);

    expandedWidget->setLayout(expandedLayout);

    // Set up the collapsed widget with its layout
    collapsedWidget = new QWidget(this);
    auto *collapsedLayout = new QHBoxLayout(collapsedWidget);
    collapsedLayout->setContentsMargins(5, 0, 5, 0);
    collapsedLayout->setSpacing(0);

    // Expand button to show full options
    expandButton = new QPushButton("▲", this);
    expandButton->setFixedSize(20, 20);
    expandButton->setToolTip("Expand");
    expandButton->setStyleSheet("border: none;");
    connect(expandButton, &QPushButton::clicked, this, &PrintingSelectorViewOptionsToolbarWidget::expand);
    collapsedLayout->addWidget(expandButton);

    // Label for collapsed state
    auto *collapsedLabel = new QLabel(tr("Display Options"), this);
    collapsedLayout->addWidget(collapsedLabel);

    collapsedWidget->setLayout(collapsedLayout);

    // Stack widget to switch between expanded and collapsed states
    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(expandedWidget);
    stackedWidget->addWidget(collapsedWidget);

    layout->addWidget(stackedWidget);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Default to the expanded widget
    stackedWidget->setCurrentWidget(expandedWidget);

    // Connect the stacked widget to update the layout when it changes
    connect(stackedWidget, &QStackedWidget::currentChanged, this,
            &PrintingSelectorViewOptionsToolbarWidget::onWidgetChanged);
}

/**
 * @brief Toggles the widget to the collapsed state.
 */
void PrintingSelectorViewOptionsToolbarWidget::collapse()
{
    stackedWidget->setCurrentWidget(collapsedWidget);
    updateGeometry();
}

/**
 * @brief Toggles the widget to the expanded state.
 */
void PrintingSelectorViewOptionsToolbarWidget::expand()
{
    stackedWidget->setCurrentWidget(expandedWidget);
    updateGeometry();
}

/**
 * @brief Handles the geometry update when the stacked widget changes.
 *
 * This ensures that the parent layout is also updated when the widget's display state changes.
 */
void PrintingSelectorViewOptionsToolbarWidget::onWidgetChanged(int)
{
    updateGeometry();
    if (parentWidget() && parentWidget()->layout()) {
        parentWidget()->layout()->invalidate();
    }
}

/**
 * @brief Provides the recommended size for the widget based on the current view.
 *
 * @return QSize The suggested size for the widget.
 */
QSize PrintingSelectorViewOptionsToolbarWidget::sizeHint() const
{
    return stackedWidget->currentWidget()->sizeHint();
}

/**
 * @brief Provides the minimum size required for the widget based on the current view.
 *
 * @return QSize The minimum size required for the widget.
 */
QSize PrintingSelectorViewOptionsToolbarWidget::minimumSizeHint() const
{
    return stackedWidget->currentWidget()->minimumSizeHint();
}

/**
 * @brief Returns the view options widget contained within this toolbar.
 *
 * @return PrintingSelectorViewOptionsWidget* The view options widget.
 */
PrintingSelectorViewOptionsWidget *PrintingSelectorViewOptionsToolbarWidget::getViewOptionsWidget() const
{
    return viewOptions;
}
