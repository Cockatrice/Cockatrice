#include "visual_database_display_tag_filter_widget.h"

#include "../../../filters/filter_tree_model.h"
#include "visual_database_display_filter_button.h"

#include <QLabel>
#include <QPointer>
#include <QScopeGuard>
#include <QSignalBlocker>
#include <libcockatrice/card/database/card_database.h>
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
    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(true);
    connect(refreshTimer, &QTimer::timeout, this, &VisualDatabaseDisplayTagFilterWidget::createTagButtons);
    auto scheduleRefresh = [this] { refreshTimer->start(0); };
    auto *database = CardDatabaseManager::getInstance();
    connect(database, &CardDatabase::cardDatabaseReset, this, scheduleRefresh);
    connect(database, &CardDatabase::cardAdded, this, scheduleRefresh);
    connect(database, &CardDatabase::cardRemoved, this, scheduleRefresh);

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
    auto scheduleSync = [this] {
        QTimer::singleShot(0, this, &VisualDatabaseDisplayTagFilterWidget::syncWithFilterModel);
    };
    connect(filterModel, &FilterTreeModel::layoutChanged, this, scheduleSync);
    connect(filterModel, &FilterTreeModel::modelReset, this, scheduleSync);

    createTagButtons(); // Populate buttons initially
    syncWithFilterModel(); // Construction must not change an already loaded filter.

    retranslateUi();
}

void VisualDatabaseDisplayTagFilterWidget::retranslateUi()
{
    searchBox->setPlaceholderText(tr("Search tags..."));
    thresholdLabel->setText(tr("Show tags with at least:"));
    spinBox->setSuffix(tr(" cards"));
    spinBox->setToolTip(tr("Do not display tags with less than this amount of cards in the database"));
    toggleButton->setToolTip(tr("Filter mode (require all selected tags, or any of them)"));
    toggleButton->setText(mixedMatchMode ? tr("Mode: Mixed")
                                         : (exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes")));
}

void VisualDatabaseDisplayTagFilterWidget::createTagButtons()
{
    allTagsWithCount = CardDatabaseManager::query()->getAllTagsWithCount();
    // Keep loaded tags visible even if the current database has no matching cards.
    if (filterModel) {
        const auto filters = filterModel->getFiltersOfType(CardFilter::AttrTag);
        for (const auto *filter : filters) {
            if (filter->type() == CardFilter::TypeAnd || filter->type() == CardFilter::TypeOr) {
                const QString tag = filter->term().trimmed().toLower();
                if (!allTagsWithCount.contains(tag)) allTagsWithCount.insert(tag, 0);
            }
        }
        qDeleteAll(filters);
    }
    tagButtons.clear();
    flowWidget->clearLayout();
    const QSignalBlocker blocker(spinBox);
    spinBox->setMaximum(getMaxTagCount());
    // Iterate through tags and create buttons.
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
    syncWithFilterModel();
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
    if (!filterModel) return;
    QPointer<VisualDatabaseDisplayTagFilterWidget> alive(this);
    QPointer<FilterTreeModel> model(filterModel);
    const auto filters = model->getFiltersOfType(CardFilter::AttrTag);
    const auto cleanup = qScopeGuard([&] { qDeleteAll(filters); });
    const auto targetType = exactMatchMode ? CardFilter::TypeAnd : CardFilter::TypeOr;
    // Only change this positive tag. Other terms, including negated tags and mixed
    // AND/OR filters loaded from disk, retain their exact semantics.
    for (const auto *filter : filters) {
        if ((filter->type() == CardFilter::TypeAnd || filter->type() == CardFilter::TypeOr) &&
            filter->term().trimmed().compare(tag, Qt::CaseInsensitive) == 0) {
            model->removeFilter(filter);
            if (!alive || !model) return;
        }
    }
    if (active) {
        const CardFilter filter(tag, targetType, CardFilter::AttrTag);
        model->addFilter(&filter);
        if (!alive || !model) return;
    }
    syncWithFilterModel();
}

void VisualDatabaseDisplayTagFilterWidget::updateFilterMode()
{
    if (!filterModel) return;
    // An explicit click converts a loaded mixed filter to all selected tags.
    exactMatchMode = mixedMatchMode ? true : !exactMatchMode;
    mixedMatchMode = false;
    const auto targetType = exactMatchMode ? CardFilter::TypeAnd : CardFilter::TypeOr;
    toggleButton->setText(mixedMatchMode ? tr("Mode: Mixed")
                                         : (exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes")));
    QPointer<VisualDatabaseDisplayTagFilterWidget> alive(this);
    QPointer<FilterTreeModel> model(filterModel);
    const auto filters = model->getFiltersOfType(CardFilter::AttrTag);
    const auto cleanup = qScopeGuard([&] { qDeleteAll(filters); });
    for (const auto *filter : filters) {
        if ((filter->type() == CardFilter::TypeAnd || filter->type() == CardFilter::TypeOr) &&
            filter->type() != targetType) {
            const CardFilter replacement(filter->term(), targetType, CardFilter::AttrTag);
            model->removeFilter(filter);
            if (!alive || !model) return;
            model->addFilter(&replacement);
            if (!alive || !model) return;
        }
    }
    syncWithFilterModel();
}

void VisualDatabaseDisplayTagFilterWidget::syncWithFilterModel()
{
    activeTags.clear();
    bool hasAnd = false;
    bool hasOr = false;
    if (filterModel) {
        const auto filters = filterModel->getFiltersOfType(CardFilter::AttrTag);
        for (const auto *filter : filters) {
            if (filter->type() == CardFilter::TypeAnd || filter->type() == CardFilter::TypeOr) {
                const QString tag = filter->term().trimmed().toLower();
                activeTags[tag] = true;
                hasAnd |= filter->type() == CardFilter::TypeAnd;
                hasOr |= filter->type() == CardFilter::TypeOr;
                // A saved custom tag may not occur in the current database.
                if (!tagButtons.contains(tag)) refreshTimer->start(0);
            }
        }
        qDeleteAll(filters);
    }
    mixedMatchMode = hasAnd && hasOr;
    if (hasAnd != hasOr) exactMatchMode = hasAnd;
    toggleButton->setText(mixedMatchMode ? tr("Mode: Mixed")
                                         : (exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes")));
    for (auto it = tagButtons.begin(); it != tagButtons.end(); ++it) {
        const QSignalBlocker blocker(it.value());
        it.value()->setChecked(activeTags.value(it.key(), false));
    }
    updateTagButtonsVisibility();
}
