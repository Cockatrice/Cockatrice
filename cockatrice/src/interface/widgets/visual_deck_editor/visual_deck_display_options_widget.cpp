#include "visual_deck_display_options_widget.h"

#include "../tabs/visual_deck_editor/tab_deck_editor_visual.h"

#include <libcockatrice/utility/qt_utils.h>

VisualDeckDisplayOptionsWidget::VisualDeckDisplayOptionsWidget(QWidget *parent) : QWidget(parent)
{
    groupAndSortLayout = new QHBoxLayout(this);
    groupAndSortLayout->setAlignment(Qt::AlignLeft);
    this->setLayout(groupAndSortLayout);

    groupByLabel = new QLabel(this);

    groupByComboBox = new QComboBox(this);

    if (auto tab = QtUtils::findParentOfType<TabDeckEditorVisual>(this)) {
        auto originalBox = tab->getDeckDockWidget()->getGroupByComboBox();
        groupByComboBox->setModel(originalBox->model());
        groupByComboBox->setModelColumn(originalBox->modelColumn());

        // Original -> clone
        connect(originalBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index) { groupByComboBox->setCurrentIndex(index); });

        // Clone -> original
        connect(groupByComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [originalBox](int index) { originalBox->setCurrentIndex(index); });
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
    sortCriteriaButton->setButtonIcon(QPixmap("theme:icons/sort_arrow_down"));

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

TutorialSequence VisualDeckDisplayOptionsWidget::generateTutorialSequence(TutorialSequence sequence)
{
    TutorialStep introStep;
    introStep.targetWidget = this;
    introStep.text = tr("You can change how the deck is displayed, grouped, and sorted here.");

    sequence.addStep(introStep);

    TutorialStep displayTypeStep;
    displayTypeStep.targetWidget = displayTypeButton;
    displayTypeStep.text =
        tr("You can change the layout of the displayed cards by clicking on this button.\n\nThe overlap type will "
           "stack cards on top of each other, leaving the top exposed for easy skimming.\nYou can always hover your "
           "mouse over a card to display a zoomed version of it.\n\nThe flat layout will display cards next to each "
           "other, without any overlap.\n\nLet's switch to flat now!");
    displayTypeStep.allowClickThrough = true;
    displayTypeStep.requiresInteraction = true;
    displayTypeStep.validationTiming = ValidationTiming::OnSignal;
    displayTypeStep.signalSource = displayTypeButton;
    displayTypeStep.signalName = SIGNAL(clicked());
    displayTypeStep.autoAdvanceOnValid = true;
    displayTypeStep.validator = [] { return true; };

    sequence.addStep(displayTypeStep);

    TutorialStep groupStep;
    groupStep.targetWidget = groupByComboBox;
    groupStep.text = tr("You can change how cards are grouped here.\n\nLet's change cards to be grouped by 'Color'");
    groupStep.allowClickThrough = true;
    groupStep.requiresInteraction = true;
    groupStep.validationTiming = ValidationTiming::OnChange;
    groupStep.autoAdvanceOnValid = true;
    groupStep.validator = [this]() { return groupByComboBox->currentIndex() == 2; };
    groupStep.validationHint = tr("Select the 'Color' option");

    sequence.addStep(groupStep);

    TutorialStep sortStep;
    sortStep.targetWidget = sortCriteriaButton;
    sortStep.text =
        tr("Let's check out sorting now. In the visual deck view, sort modifiers are hierarchical,\n meaning "
           "that the cards will first be sorted using the top-most criteria\nand then, if cards are equal using this "
           "criteria,\nthe next criteria in the list will be used as a tie-breaker.\n\n"
           "Change the sorting to be based primarily on converted mana cost (cmc) by dragging it to the top.");
    sortStep.allowClickThrough = true;
    sortStep.requiresInteraction = true;
    sortStep.autoAdvanceOnValid = true;
    sortStep.validationTiming = ValidationTiming::OnSignal;
    sortStep.signalSource = this;
    sortStep.signalName = SIGNAL(sortCriteriaChanged(const QStringList &));
    sortStep.validator = []() { return true; };

    sequence.addStep(sortStep);

    return sequence;
}
