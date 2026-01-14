#include "visual_database_display_filter_toolbar_widget.h"

#include "visual_database_display_widget.h"

VisualDatabaseDisplayFilterToolbarWidget::VisualDatabaseDisplayFilterToolbarWidget(VisualDatabaseDisplayWidget *_parent)
    : QWidget(_parent), visualDatabaseDisplay(_parent)
{
    filterContainerLayout = new QHBoxLayout(this);
    // filterContainerLayout->setContentsMargins(11, 0, 11, 0);
    setLayout(filterContainerLayout);
    filterContainerLayout->setAlignment(Qt::AlignLeft);

    setMaximumHeight(80);

    connect(this, &VisualDatabaseDisplayFilterToolbarWidget::searchModelChanged, visualDatabaseDisplay,
            &VisualDatabaseDisplayWidget::onSearchModelChanged);

    filterByLabel = new QLabel(this);

    sortByLabel = new QLabel(this);
    sortColumnCombo = new QComboBox(this);
    sortColumnCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
    sortOrderCombo = new QComboBox(this);
    sortOrderCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

    sortOrderCombo->addItem("Ascending", Qt::AscendingOrder);
    sortOrderCombo->addItem("Descending", Qt::DescendingOrder);
    sortOrderCombo->view()->setMinimumWidth(sortOrderCombo->view()->sizeHintForColumn(0));
    sortOrderCombo->adjustSize();

    // Populate columns dynamically from the model
    for (int i = 0; i < visualDatabaseDisplay->getDatabaseDisplayModel()->columnCount(); ++i) {
        QString header = visualDatabaseDisplay->getDatabaseDisplayModel()->headerData(i, Qt::Horizontal).toString();
        sortColumnCombo->addItem(header, i);
    }

    sortColumnCombo->view()->setMinimumWidth(sortColumnCombo->view()->sizeHintForColumn(0));
    sortColumnCombo->adjustSize();

    connect(sortColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        int column = sortColumnCombo->currentData().toInt();
        Qt::SortOrder order = static_cast<Qt::SortOrder>(sortOrderCombo->currentData().toInt());
        visualDatabaseDisplay->getDatabaseView()->sortByColumn(column, order);

        emit searchModelChanged();
    });

    connect(sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        int column = sortColumnCombo->currentData().toInt();
        Qt::SortOrder order = static_cast<Qt::SortOrder>(sortOrderCombo->currentData().toInt());
        visualDatabaseDisplay->getDatabaseView()->sortByColumn(column, order);

        emit searchModelChanged();
    });

    quickFilterSaveLoadWidget = new SettingsButtonWidget(this);
    quickFilterSaveLoadWidget->setButtonIcon(QPixmap("theme:icons/floppy_disk"));

    quickFilterNameWidget = new SettingsButtonWidget(this);
    quickFilterNameWidget->setButtonIcon(QPixmap("theme:icons/pen_to_square"));

    quickFilterMainTypeWidget = new SettingsButtonWidget(this);
    quickFilterMainTypeWidget->setButtonIcon(QPixmap("theme:icons/circle_half_stroke"));

    quickFilterSubTypeWidget = new SettingsButtonWidget(this);
    quickFilterSubTypeWidget->setButtonIcon(QPixmap("theme:icons/dragon"));

    quickFilterSetWidget = new SettingsButtonWidget(this);
    quickFilterSetWidget->setButtonIcon(QPixmap("theme:icons/scroll"));

    quickFilterFormatLegalityWidget = new SettingsButtonWidget(this);
    quickFilterFormatLegalityWidget->setButtonIcon(QPixmap("theme:icons/scale_balanced"));

    retranslateUi();
}

void VisualDatabaseDisplayFilterToolbarWidget::initialize()
{
    sortByLabel->setVisible(true);
    filterByLabel->setVisible(true);

    quickFilterSaveLoadWidget->setVisible(true);
    quickFilterNameWidget->setVisible(true);
    quickFilterSubTypeWidget->setVisible(true);
    quickFilterSetWidget->setVisible(true);

    auto filterModel = visualDatabaseDisplay->filterModel;

    saveLoadWidget = new VisualDatabaseDisplayFilterSaveLoadWidget(this, filterModel);
    nameFilterWidget =
        new VisualDatabaseDisplayNameFilterWidget(this, visualDatabaseDisplay->getDeckEditor(), filterModel);
    mainTypeFilterWidget = new VisualDatabaseDisplayMainTypeFilterWidget(this, filterModel);
    formatLegalityWidget = new VisualDatabaseDisplayFormatLegalityFilterWidget(this, filterModel);
    subTypeFilterWidget = new VisualDatabaseDisplaySubTypeFilterWidget(this, filterModel);
    setFilterWidget = new VisualDatabaseDisplaySetFilterWidget(this, filterModel);

    quickFilterSaveLoadWidget->addSettingsWidget(saveLoadWidget);
    quickFilterNameWidget->addSettingsWidget(nameFilterWidget);
    quickFilterMainTypeWidget->addSettingsWidget(mainTypeFilterWidget);
    quickFilterSubTypeWidget->addSettingsWidget(subTypeFilterWidget);
    quickFilterSetWidget->addSettingsWidget(setFilterWidget);
    quickFilterFormatLegalityWidget->addSettingsWidget(formatLegalityWidget);

    filterContainerLayout->addWidget(sortByLabel);
    filterContainerLayout->addWidget(sortColumnCombo);
    filterContainerLayout->addWidget(sortOrderCombo);
    filterContainerLayout->addWidget(filterByLabel);
    filterContainerLayout->addWidget(quickFilterNameWidget);
    filterContainerLayout->addWidget(quickFilterMainTypeWidget);
    filterContainerLayout->addWidget(quickFilterSubTypeWidget);
    filterContainerLayout->addWidget(quickFilterSetWidget);
    filterContainerLayout->addWidget(quickFilterFormatLegalityWidget);
    filterContainerLayout->addStretch();
    filterContainerLayout->addWidget(quickFilterSaveLoadWidget);
}

void VisualDatabaseDisplayFilterToolbarWidget::retranslateUi()
{
    sortByLabel->setText(tr("Sort by:"));
    filterByLabel->setText(tr("Filter by:"));

    quickFilterSaveLoadWidget->setToolTip(tr("Save and load filters"));
    quickFilterNameWidget->setToolTip(tr("Filter by exact card name"));
    quickFilterMainTypeWidget->setToolTip(tr("Filter by card main-type"));
    quickFilterSubTypeWidget->setToolTip(tr("Filter by card sub-type"));
    quickFilterSetWidget->setToolTip(tr("Filter by set"));
    quickFilterFormatLegalityWidget->setToolTip(tr("Filter by format legality"));

    quickFilterSaveLoadWidget->setButtonText(tr("Save/Load"));
    quickFilterNameWidget->setButtonText(tr("Name"));
    quickFilterMainTypeWidget->setButtonText(tr("Main Type"));
    quickFilterSubTypeWidget->setButtonText(tr("Sub Type"));
    quickFilterSetWidget->setButtonText(tr("Sets"));
    quickFilterFormatLegalityWidget->setButtonText(tr("Formats"));
}