#include "mana_base_config_dialog.h"

#include <QPushButton>

ManaBaseConfigDialog::ManaBaseConfigDialog(DeckListStatisticsAnalyzer *analyzer,
                                           ManaBaseConfig initial,
                                           QWidget *parent)
    : QDialog(parent), config(initial)
{
    layout = new QVBoxLayout(this);

    displayTypeLabel = new QLabel(this);
    layout->addWidget(displayTypeLabel);

    displayType = new QComboBox(this);
    layout->addWidget(displayType);

    filterLabel = new QLabel(this);
    layout->addWidget(filterLabel);

    filterList = new QListWidget(this);
    filterList->setSelectionMode(QAbstractItemView::MultiSelection);
    layout->addWidget(filterList);

    QStringList colors = analyzer->getManaBase().keys();
    colors.sort();
    filterList->addItems(colors);

    // select initial filters
    for (int i = 0; i < filterList->count(); ++i) {
        if (config.filters.contains(filterList->item(i)->text()))
            filterList->item(i)->setSelected(true);
    }

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &ManaBaseConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ManaBaseConfigDialog::reject);

    retranslateUi();
}

void ManaBaseConfigDialog::retranslateUi()
{
    setWindowTitle(tr("Mana Base Configuration"));

    displayTypeLabel->setText(tr("Display type:"));

    displayType->clear();
    displayType->addItems({tr("pie"), tr("bar"), tr("combinedBar")});

    filterLabel->setText(tr("Filter Colors (optional):"));

    buttons->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

void ManaBaseConfigDialog::accept()
{
    config.displayType = displayType->currentText();
    config.filters.clear();
    for (auto *item : filterList->selectedItems()) {
        config.filters << item->text();
    }
    QDialog::accept();
}
