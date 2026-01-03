#include "mana_devotion_config_dialog.h"

ManaDevotionConfigDialog::ManaDevotionConfigDialog(DeckListStatisticsAnalyzer *analyzer,
                                                   ManaDevotionConfig initial,
                                                   QWidget *parent)
    : QDialog(parent), config(initial)
{
    layout = new QVBoxLayout(this);

    labelDisplayType = new QLabel(this);
    layout->addWidget(labelDisplayType);

    displayType = new QComboBox(this);
    layout->addWidget(displayType);

    labelFilters = new QLabel(this);
    layout->addWidget(labelFilters);

    filterList = new QListWidget(this);
    filterList->setSelectionMode(QAbstractItemView::MultiSelection);

    QStringList colors = analyzer->getDevotionPipCount().keys();
    colors.sort();
    filterList->addItems(colors);
    layout->addWidget(filterList);

    // select initial filters
    for (int i = 0; i < filterList->count(); ++i) {
        if (config.filters.contains(filterList->item(i)->text()))
            filterList->item(i)->setSelected(true);
    }

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ManaDevotionConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ManaDevotionConfigDialog::reject);

    // populate combo box items
    displayType->addItems({"pie", "bar", "combinedBar"});

    retranslateUi();
}

void ManaDevotionConfigDialog::retranslateUi()
{
    labelDisplayType->setText(tr("Display type:"));
    displayType->setItemText(0, tr("pie"));
    displayType->setItemText(1, tr("bar"));
    displayType->setItemText(2, tr("combinedBar"));

    labelFilters->setText(tr("Filter Colors (optional):"));
}

void ManaDevotionConfigDialog::accept()
{
    config.displayType = displayType->currentText();
    config.filters.clear();
    for (auto *item : filterList->selectedItems()) {
        config.filters << item->text();
    }
    QDialog::accept();
}
