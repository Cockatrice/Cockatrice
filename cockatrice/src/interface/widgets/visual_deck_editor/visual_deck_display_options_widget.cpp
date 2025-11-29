#include "visual_deck_display_options_widget.h"

#include "../tabs/visual_deck_editor/tab_deck_editor_visual.h"

VisualDeckDisplayOptionsWidget::VisualDeckDisplayOptionsWidget(QWidget *parent)
{
    groupAndSortLayout = new QHBoxLayout(this);
    groupAndSortLayout->setAlignment(Qt::AlignLeft);
    this->setLayout(groupAndSortLayout);

    groupByLabel = new QLabel(this);

    groupByComboBox = new QComboBox(this);
    if (auto visualDeckEditorWidget = qobject_cast<VisualDeckEditorWidget *>(parent)) {
        if (auto tabWidget = qobject_cast<TabDeckEditorVisualTabWidget *>(visualDeckEditorWidget)) {
            // Inside a central widget QWidget container inside TabDeckEditorVisual
            if (auto tab = qobject_cast<TabDeckEditorVisual *>(tabWidget->parent()->parent())) {
                auto originalBox = tab->getDeckDockWidget()->getGroupByComboBox();
                groupByComboBox->setModel(originalBox->model());
                groupByComboBox->setModelColumn(originalBox->modelColumn());

                // Original -> clone
                connect(originalBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [this](int index) { groupByComboBox->setCurrentIndex(index); });

                // Clone -> original
                connect(groupByComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        [originalBox](int index) { originalBox->setCurrentIndex(index); });
            }
        }
    } else {
        groupByComboBox->addItem(
            tr(qPrintable(DeckListModelGroupCriteria::toString(DeckListModelGroupCriteria::MAIN_TYPE))),
            DeckListModelGroupCriteria::MAIN_TYPE);
        groupByComboBox->addItem(
            tr(qPrintable(DeckListModelGroupCriteria::toString(DeckListModelGroupCriteria::MANA_COST))),
            DeckListModelGroupCriteria::MANA_COST);
        groupByComboBox->addItem(
            tr(qPrintable(DeckListModelGroupCriteria::toString(DeckListModelGroupCriteria::COLOR))),
            DeckListModelGroupCriteria::COLOR);
        groupByComboBox->setMinimumWidth(300);
        connect(groupByComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
                &VisualDeckDisplayOptionsWidget::groupCriteriaChanged);
        emit groupCriteriaChanged(groupByComboBox->currentText());
    }

    sortByLabel = new QLabel(this);

    sortCriteriaButton = new SettingsButtonWidget(this);

    sortLabel = new QLabel(sortCriteriaButton);
    sortLabel->setWordWrap(true);

    QStringList sortProperties = {"colors", "cmc", "name", "maintype"};
    sortByListWidget = new QListWidget();
    sortByListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    sortByListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    sortByListWidget->setDefaultDropAction(Qt::MoveAction);

    for (const QString &property : sortProperties) {
        QListWidgetItem *item = new QListWidgetItem(property, sortByListWidget);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    connect(sortByListWidget->model(), &QAbstractItemModel::rowsMoved, this,
            &VisualDeckDisplayOptionsWidget::onSortCriteriaChange);
    onSortCriteriaChange();

    sortByListWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    sortCriteriaButton->addSettingsWidget(sortLabel);
    sortCriteriaButton->addSettingsWidget(sortByListWidget);

    displayTypeButton = new QPushButton(this);
    connect(displayTypeButton, &QPushButton::clicked, this, &VisualDeckDisplayOptionsWidget::updateDisplayType);

    groupAndSortLayout->addWidget(groupByLabel);
    groupAndSortLayout->addWidget(groupByComboBox);
    groupAndSortLayout->addWidget(sortByLabel);
    groupAndSortLayout->addWidget(sortCriteriaButton);
    groupAndSortLayout->addWidget(displayTypeButton);

    retranslateUi();
}

void VisualDeckDisplayOptionsWidget::retranslateUi()
{
    groupByLabel->setText(tr("Group by:"));
    groupByComboBox->setToolTip(tr("Change how cards are divided into categories/groups."));
    sortByLabel->setText(tr("Sort by:"));
    sortLabel->setText(tr("Click and drag to change the sort order within the groups"));
    sortCriteriaButton->setToolTip(tr("Configure how cards are sorted within their groups"));
    displayTypeButton->setText(tr("Toggle Layout: Overlap"));
    displayTypeButton->setToolTip(
        tr("Change how cards are displayed within zones (i.e. overlapped or fully visible.)"));
}

void VisualDeckDisplayOptionsWidget::onSortCriteriaChange()
{
    QStringList selectedCriteria;
    for (int i = 0; i < sortByListWidget->count(); ++i) {
        QListWidgetItem *item = sortByListWidget->item(i);
        selectedCriteria.append(item->text()); // Collect user-defined sort order
    }

    emit sortCriteriaChanged(selectedCriteria);
}

void VisualDeckDisplayOptionsWidget::updateDisplayType()
{
    // Toggle the display type
    currentDisplayType = (currentDisplayType == DisplayType::Overlap) ? DisplayType::Flat : DisplayType::Overlap;

    // Update UI and emit signal
    switch (currentDisplayType) {
        case DisplayType::Flat:
            displayTypeButton->setText(tr("Toggle Layout: Flat"));
            break;
        case DisplayType::Overlap:
            displayTypeButton->setText(tr("Toggle Layout: Overlap"));
            break;
    }
    emit displayTypeChanged(currentDisplayType);
}
