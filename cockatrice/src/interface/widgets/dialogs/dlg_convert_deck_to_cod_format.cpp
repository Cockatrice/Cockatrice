#include "dlg_convert_deck_to_cod_format.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

DialogConvertDeckToCodFormat::DialogConvertDeckToCodFormat(QWidget *parent) : QDialog(parent)
{
    layout = new QVBoxLayout(this);
    label = new QLabel();
    layout->addWidget(label);

    dontAskAgainCheckbox = new QCheckBox(this);
    layout->addWidget(dontAskAgainCheckbox);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() { accept(); });

    connect(buttonBox, &QDialogButtonBox::rejected, this, [this]() { reject(); });

    setLayout(layout);
    retranslateUi();
}

void DialogConvertDeckToCodFormat::retranslateUi()
{
    setWindowTitle(tr("Deck Format Conversion"));
    label->setText(
        tr("You tried to add a tag to a .txt format deck.\n Tags can only be added to .cod format decks.\n Do "
           "you want to convert the deck to the .cod format?"));
    dontAskAgainCheckbox->setText(tr("Remember and automatically apply choice in the future"));
}

bool DialogConvertDeckToCodFormat::dontAskAgain() const
{
    return dontAskAgainCheckbox->isChecked();
}
