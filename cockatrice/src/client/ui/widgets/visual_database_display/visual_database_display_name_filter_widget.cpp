#include "visual_database_display_name_filter_widget.h"

#include <QHBoxLayout>

VisualDatabaseDisplayNameFilterWidget::VisualDatabaseDisplayNameFilterWidget(QWidget *parent,
                                                                             FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Create the search box
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(tr("Filter by name..."));
    layout->addWidget(searchBox);

    connect(searchBox, &QLineEdit::returnPressed, this, [this]() {
        QString text = searchBox->text().trimmed();
        if (!text.isEmpty() && !activeFilters.contains(text)) {
            createNameFilter(text);
            searchBox->clear();
        }
    });

    // Create container for active filters
    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);
}

void VisualDatabaseDisplayNameFilterWidget::createNameFilter(const QString &name)
{
    if (activeFilters.contains(name))
        return;

    // Create a button for the filter
    auto *button = new QPushButton(name, flowWidget);
    button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                          "QPushButton:hover { background-color: red; color: white; }");

    connect(button, &QPushButton::clicked, this, [this, name]() { removeNameFilter(name); });

    flowWidget->addWidget(button);
    activeFilters[name] = button;

    updateFilterModel();
}

void VisualDatabaseDisplayNameFilterWidget::removeNameFilter(const QString &name)
{
    if (activeFilters.contains(name)) {
        delete activeFilters[name]; // Remove button
        activeFilters.remove(name);
        updateFilterModel();
    }
}

void VisualDatabaseDisplayNameFilterWidget::updateFilterModel()
{
    // Clear existing name filters
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrName);

    for (const auto &name : activeFilters.keys()) {
        QString nameString = name;
        filterModel->addFilter(new CardFilter(nameString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrName));
    }
}
