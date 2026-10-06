#include "visual_database_display_tag_filter_widget.h"

#include "../../../filters/filter_tree_model.h"
#include "visual_database_display_filter_button.h"

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/filters/filter_tree.h>

VisualDatabaseDisplayTagFilterWidget::VisualDatabaseDisplayTagFilterWidget(QWidget *parent,
                                                                           FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allTagsWithCount = CardDatabaseManager::query()->getAllTagsWithCount();

    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 1, 0, 1);
    layout->setSpacing(1);
    layout->setAlignment(Qt::AlignTop);

    // Create search box
    searchBox = new QLineEdit(this);
    layout->addWidget(searchBox);
    connect(searchBox, &QLineEdit::textChanged, this,
            &VisualDatabaseDisplayTagFilterWidget::updateTagButtonsVisibility);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    // Create a container for the threshold control
    auto *thresholdLayout = new QHBoxLayout();
    thresholdLayout->setContentsMargins(0, 0, 0, 0);

    thresholdLabel = new QLabel(this);
    thresholdLayout->addWidget(thresholdLabel);

    // Create the spinbox
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setMaximum(getMaxTagCount());
    spinBox->setValue(150);
    thresholdLayout->addWidget(spinBox);
    thresholdLayout->addStretch();

    layout->addLayout(thresholdLayout);

    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplayTagFilterWidget::updateTagButtonsVisibility);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::clicked, this, &VisualDatabaseDisplayTagFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { QTimer::singleShot(100, this, &VisualDatabaseDisplayTagFilterWidget::syncWithFilterModel); });

    createTagButtons(); // Populate buttons initially
    updateFilterMode(); // Initialize toggle button text

    retranslateUi();
}

void VisualDatabaseDisplayTagFilterWidget::retranslateUi()
{
    searchBox->setPlaceholderText(tr("Search tags..."));
    thresholdLabel->setText(tr("Show tags with at least:"));
    spinBox->setSuffix(tr(" cards"));
    spinBox->setToolTip(tr("Do not display tags with less than this amount of cards in the database"));
    toggleButton->setToolTip(tr("Filter mode (require all selected tags, or any of them)"));
}

void VisualDatabaseDisplayTagFilterWidget::createTagButtons()
{
    // Iterate through tags and create buttons
    for (auto it = allTagsWithCount.begin(); it != allTagsWithCount.end(); ++it) {
        auto *button = new QPushButton(it.key(), flowWidget);
        button->setCheckable(true);
        button->setStyleSheet(visualDatabaseDisplayFilterButtonStyle);

        flowWidget->addWidget(button);
        tagButtons[it.key()] = button;

        // Connect toggle signal for each button
        connect(button, &QPushButton::toggled, this,
                [this, tag = it.key()](bool checked) { handleTagToggled(tag, checked); });
    }
    updateTagButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplayTagFilterWidget::updateTagButtonsVisibility()
{
    const int threshold = spinBox->value();
    const QString filterText = searchBox->text().trimmed().toLower();

    // Iterate through buttons and hide/disable those below the threshold. An
    // active search overrides the threshold so rare tags stay reachable.
    for (auto it = tagButtons.begin(); it != tagButtons.end(); ++it) {
        const bool isActive = activeTags.value(it.key(), false);
        const bool visible = isActive || (filterText.isEmpty() ? allTagsWithCount[it.key()] >= threshold
                                                               : it.key().toLower().contains(filterText));

        it.value()->setVisible(visible);
        it.value()->setEnabled(visible);
    }
}

int VisualDatabaseDisplayTagFilterWidget::getMaxTagCount() const
{
    int maxCount = 1;
    for (auto it = allTagsWithCount.begin(); it != allTagsWithCount.end(); ++it) {
        maxCount = qMax(maxCount, it.value());
    }
    return maxCount;
}

void VisualDatabaseDisplayTagFilterWidget::handleTagToggled(const QString &tag, bool active)
{
    activeTags[tag] = active;

    if (tagButtons.contains(tag)) {
        tagButtons[tag]->setChecked(active);
    }

    updateTagFilter();
}

void VisualDatabaseDisplayTagFilterWidget::updateTagFilter()
{
    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrTag);

    for (const auto &activeTag : activeTags.keys()) {
        if (activeTags[activeTag]) {
            QString tag = activeTag;
            filterModel->addFilter(new CardFilter(
                tag, exactMatchMode ? CardFilter::Type::TypeAnd : CardFilter::Type::TypeOr, CardFilter::Attr::AttrTag));
        }
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayTagFilterWidget::updateFilterMode()
{
    exactMatchMode = !exactMatchMode;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateTagFilter();
}

void VisualDatabaseDisplayTagFilterWidget::syncWithFilterModel()
{
    // Temporarily block signals for each button to prevent toggling while updating button states
    for (auto it = tagButtons.begin(); it != tagButtons.end(); ++it) {
        it.value()->blockSignals(true);
    }

    // Uncheck all buttons
    for (auto it = tagButtons.begin(); it != tagButtons.end(); ++it) {
        it.value()->setChecked(false);
    }

    // Get active filters for tags
    QSet<QString> activeTypes;
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::AttrTag)) {
        if (filter->type() == CardFilter::Type::TypeAnd || filter->type() == CardFilter::Type::TypeOr) {
            activeTypes.insert(filter->term());
        }
    }

    // Check the buttons for active tags
    for (const auto &tag : activeTypes) {
        activeTags[tag] = true;
        if (tagButtons.contains(tag)) {
            tagButtons[tag]->setChecked(true);
        }
    }

    // Re-enable signal emissions for each button
    for (auto it = tagButtons.begin(); it != tagButtons.end(); ++it) {
        it.value()->blockSignals(false);
    }

    // Update the visibility of buttons
    updateTagButtonsVisibility();
}
