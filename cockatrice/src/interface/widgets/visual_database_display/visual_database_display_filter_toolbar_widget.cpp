#include "visual_database_display_filter_toolbar_widget.h"

#include "visual_database_display_widget.h"

#include <QGroupBox>

VisualDatabaseDisplayFilterToolbarWidget::VisualDatabaseDisplayFilterToolbarWidget(VisualDatabaseDisplayWidget *_parent)
    : QWidget(_parent), visualDatabaseDisplay(_parent)
{
    filterContainerLayout = new QHBoxLayout(this);
    filterContainerLayout->setContentsMargins(11, 0, 11, 0);
    filterContainerLayout->setSpacing(2);
    setLayout(filterContainerLayout);
    filterContainerLayout->setAlignment(Qt::AlignLeft);

    setMaximumHeight(80);

    connect(this, &VisualDatabaseDisplayFilterToolbarWidget::searchModelChanged, visualDatabaseDisplay,
            &VisualDatabaseDisplayWidget::onSearchModelChanged);

    sortGroupBox = new QGroupBox(this);
    filterGroupBox = new QGroupBox(this);

    auto scalePixmap = [](const QString &fileName) { return QIcon(QPixmap(fileName)).pixmap({20, 20}); };

    sortByLabel = new QLabel(this);
    sortByLabel->setPixmap(scalePixmap("theme:icons/sort_arrow_down"));

    filterByLabel = new QLabel(this);
    filterByLabel->setPixmap(scalePixmap("theme:icons/filter"));

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
    // create groupbox layouts
    auto sortLayout = new QHBoxLayout(this);
    sortLayout->setContentsMargins(0, 0, 0, 0);
    sortLayout->setSpacing(0);
    sortGroupBox->setLayout(sortLayout);
    sortLayout->setAlignment(Qt::AlignLeft);

    auto filterLayout = new QHBoxLayout(this);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(2);
    filterGroupBox->setLayout(filterLayout);
    filterLayout->setAlignment(Qt::AlignLeft);

    // create settings widgets
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

    // fill groupbox layouts
    sortLayout->addWidget(sortByLabel);
    sortLayout->addWidget(sortColumnCombo);
    sortLayout->addWidget(sortOrderCombo);

    filterLayout->addWidget(filterByLabel);
    filterLayout->addWidget(quickFilterNameWidget);
    filterLayout->addWidget(quickFilterMainTypeWidget);
    filterLayout->addWidget(quickFilterSubTypeWidget);
    filterLayout->addWidget(quickFilterSetWidget);
    filterLayout->addWidget(quickFilterFormatLegalityWidget);

    // put everything into main layout
    filterContainerLayout->addWidget(sortGroupBox);
    filterContainerLayout->addWidget(filterGroupBox);
    filterContainerLayout->addStretch();
    filterContainerLayout->addWidget(quickFilterSaveLoadWidget);
}

void VisualDatabaseDisplayFilterToolbarWidget::retranslateUi()
{
    sortByLabel->setToolTip(tr("Sort by"));
    filterByLabel->setToolTip(tr("Filter by"));

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