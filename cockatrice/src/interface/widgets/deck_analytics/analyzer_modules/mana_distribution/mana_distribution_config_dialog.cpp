#include "mana_distribution_config_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

static const QStringList kColors = {"W", "U", "B", "R", "G", "C"};

ManaDistributionConfigDialog::ManaDistributionConfigDialog(DeckListStatisticsAnalyzer *analyzer, QWidget *parent)
    : QDialog(parent), analyzer(analyzer)
{
    auto *lay = new QVBoxLayout(this);

    // Labels
    labelDisplayType = new QLabel(this);
    lay->addWidget(labelDisplayType);

    displayType = new QComboBox(this);
    lay->addWidget(displayType);

    labelFilters = new QLabel(this);
    lay->addWidget(labelFilters);

    filterList = new QListWidget(this);
    filterList->setSelectionMode(QAbstractItemView::MultiSelection);
    filterList->addItems(kColors); // dynamic/fixed, no translation needed
    lay->addWidget(filterList);

    showColorRows = new QCheckBox(this);
    showColorRows->setChecked(true);
    lay->addWidget(showColorRows);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ManaDistributionConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ManaDistributionConfigDialog::reject);

    displayType->addItems({"pie", "bar"}); // combo items

    retranslateUi();
}

void ManaDistributionConfigDialog::retranslateUi()
{
    labelDisplayType->setText(tr("Top display type:"));
    displayType->setItemText(0, tr("pie"));
    displayType->setItemText(1, tr("bar"));

    labelFilters->setText(tr("Colors:"));

    showColorRows->setText(tr("Show per-color rows"));

    // QDialogButtonBox buttons are automatically translated
}

void ManaDistributionConfigDialog::setFromConfig(const ManaDistributionConfig &cfgIn)
{
    cfg = cfgIn;

    displayType->setCurrentText(cfg.displayType);

    for (int i = 0; i < filterList->count(); ++i)
        filterList->item(i)->setSelected(cfg.filters.contains(filterList->item(i)->text()));

    showColorRows->setChecked(cfg.showColorRows);
}

void ManaDistributionConfigDialog::accept()
{
    cfg.displayType = displayType->currentText();

    // Filters
    cfg.filters.clear();
    for (auto *item : filterList->selectedItems())
        cfg.filters << item->text();

    cfg.showColorRows = showColorRows->isChecked();

    QDialog::accept();
}
