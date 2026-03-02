#include "draw_probability_config_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>

DrawProbabilityConfigDialog::DrawProbabilityConfigDialog(QWidget *parent) : QDialog(parent)
{
    form = new QFormLayout(this);

    // Criteria
    labelCriteria = new QLabel(this);
    criteria = new QComboBox(this);
    criteria->addItem(QString(), "name");
    criteria->addItem(QString(), "type");
    criteria->addItem(QString(), "subtype");
    criteria->addItem(QString(), "cmc");
    form->addRow(labelCriteria, criteria);

    // Exactness
    labelExactness = new QLabel(this);
    exactness = new QComboBox(this);
    exactness->addItem(QString(), true);
    exactness->addItem(QString(), false);
    form->addRow(labelExactness, exactness);

    // Quantity
    labelQuantity = new QLabel(this);
    quantity = new QSpinBox(this);
    quantity->setRange(1, 60);
    form->addRow(labelQuantity, quantity);

    // Drawn
    labelDrawn = new QLabel(this);
    drawn = new QSpinBox(this);
    drawn->setRange(1, 60);
    drawn->setValue(7);
    form->addRow(labelDrawn, drawn);

    // Button box
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addWidget(bb);

    connect(bb, &QDialogButtonBox::accepted, this, &DrawProbabilityConfigDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    retranslateUi();
}

void DrawProbabilityConfigDialog::retranslateUi()
{
    setWindowTitle(tr("Draw Probability Settings"));

    labelCriteria->setText(tr("Criteria:"));
    criteria->setItemText(0, tr("Card Name"));
    criteria->setItemText(1, tr("Type"));
    criteria->setItemText(2, tr("Subtype"));
    criteria->setItemText(3, tr("Mana Value"));

    labelExactness->setText(tr("Exactness:"));
    exactness->setItemText(0, tr("At least"));
    exactness->setItemText(1, tr("Exactly"));

    labelQuantity->setText(tr("Quantity (N):"));
    labelDrawn->setText(tr("Cards drawn (M):"));

    // i18n-friendly suffixes
    quantity->setSuffix(tr(" cards"));
    drawn->setSuffix(tr(" cards"));
}

void DrawProbabilityConfigDialog::setFromConfig(const DrawProbabilityConfig &_config)
{
    cfg = _config;

    criteria->setCurrentIndex(criteria->findData(_config.criteria));
    exactness->setCurrentIndex(exactness->findData(_config.atLeast));
    quantity->setValue(_config.quantity);
    drawn->setValue(_config.drawn);
}

void DrawProbabilityConfigDialog::accept()
{
    cfg.criteria = criteria->currentData().toString();
    cfg.atLeast = exactness->currentData().toBool();
    cfg.quantity = quantity->value();
    cfg.drawn = drawn->value();

    QDialog::accept();
}
