#include "deck_preview_tag_deck_format_conversion_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

DeckPreviewTagDeckFormatConversionDialog::DeckPreviewTagDeckFormatConversionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Deck Format Conversion");
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

void DeckPreviewTagDeckFormatConversionDialog::retranslateUi()
{
    label->setText(tr("You tried to add a tag to a .txt format deck. Tags can only be added to .cod format decks. Do "
                      "you want to convert the deck to the .cod format?"));
    dontAskAgainCheckbox->setText(tr("Remember and automatically apply choice in the future"));
}

bool DeckPreviewTagDeckFormatConversionDialog::dontAskAgain() const
{
    return dontAskAgainCheckbox->isChecked();
}
