#include "mana_curve_config_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

ManaCurveConfigDialog::ManaCurveConfigDialog(DeckListStatisticsAnalyzer *analyzer, QWidget *parent)
    : QDialog(parent), analyzer(analyzer)
{
    auto *lay = new QVBoxLayout(this);

    labelGroupBy = new QLabel(this);
    lay->addWidget(labelGroupBy);

    groupBy = new QComboBox(this);
    lay->addWidget(groupBy);

    labelFilters = new QLabel(this);
    lay->addWidget(labelFilters);

    filterList = new QListWidget(this);
    filterList->setSelectionMode(QAbstractItemView::MultiSelection);
    lay->addWidget(filterList);

    showMain = new QCheckBox(this);
    showMain->setChecked(true);
    lay->addWidget(showMain);

    showCatRows = new QCheckBox(this);
    showCatRows->setChecked(true);
    lay->addWidget(showCatRows);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ManaCurveConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ManaCurveConfigDialog::reject);

    // populate dynamic data
    QStringList cats = analyzer->getManaCurveByType().keys();
    cats.append(analyzer->getManaCurveByColor().keys());
    cats.removeDuplicates();
    cats.sort();
    filterList->addItems(cats);

    groupBy->addItems({"type", "color", "subtype", "power", "toughness"});

    retranslateUi();
}

void ManaCurveConfigDialog::retranslateUi()
{
    labelGroupBy->setText(tr("Group By:"));
    groupBy->setItemText(0, tr("type"));
    groupBy->setItemText(1, tr("color"));
    groupBy->setItemText(2, tr("subtype"));
    groupBy->setItemText(3, tr("power"));
    groupBy->setItemText(4, tr("toughness"));

    labelFilters->setText(tr("Filters (optional):"));

    showMain->setText(tr("Show main bar row"));
    showCatRows->setText(tr("Show per-category rows"));
}

void ManaCurveConfigDialog::setFromConfig(const ManaCurveConfig &cfg)
{
    groupBy->setCurrentText(cfg.groupBy);
    // restore filters
    for (int i = 0; i < filterList->count(); ++i)
        filterList->item(i)->setSelected(cfg.filters.contains(filterList->item(i)->text()));

    showMain->setChecked(cfg.showMain);
    showCatRows->setChecked(cfg.showCategoryRows);
}

void ManaCurveConfigDialog::accept()
{
    cfg.groupBy = groupBy->currentText();

    cfg.filters.clear();
    for (auto *item : filterList->selectedItems())
        cfg.filters << item->text();

    cfg.showMain = showMain->isChecked();
    cfg.showCategoryRows = showCatRows->isChecked();

    QDialog::accept();
}
