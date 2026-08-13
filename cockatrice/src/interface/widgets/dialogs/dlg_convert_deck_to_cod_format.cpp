#include "dlg_convert_deck_to_cod_format.h"

#include "../../../client/settings/cache_settings.h"
#include "../../deck_loader/deck_loader.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

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

static bool confirmOverwriteIfExists(QWidget *parent, const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString newFileName = QDir::toNativeSeparators(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".cod");

    if (QFile::exists(newFileName)) {
        QMessageBox::StandardButton reply =
            QMessageBox::question(parent, QObject::tr("Overwrite Existing File?"),
                                  QObject::tr("A .cod version of this deck already exists. Overwrite it?"),
                                  QMessageBox::Yes | QMessageBox::No);
        return reply == QMessageBox::Yes;
    }
    return true; // Safe to proceed
}

bool promptFileConversionIfRequired(QWidget *parent, const QString &filePath, const std::function<bool()> &convert)
{
    if (DeckFileFormat::getFormatFromName(filePath) == DeckFileFormat::Cockatrice) {
        return true;
    }

    // Retrieve saved preference if the prompt is disabled
    if (!SettingsCache::instance().visualDeckStorage().getVisualDeckStoragePromptForConversion()) {
        if (!SettingsCache::instance().visualDeckStorage().getVisualDeckStorageAlwaysConvert()) {
            return false;
        }

        if (!confirmOverwriteIfExists(parent, filePath)) {
            return false;
        }

        return convert();
    }

    // Show the dialog to the user
    DialogConvertDeckToCodFormat conversionDialog(parent);
    if (conversionDialog.exec() != QDialog::Accepted) {
        SettingsCache::instance().visualDeckStorage().setVisualDeckStoragePromptForConversion(
            !conversionDialog.dontAskAgain());
        SettingsCache::instance().visualDeckStorage().setVisualDeckStorageAlwaysConvert(false);

        return false;
    }

    // Try to convert file
    if (!confirmOverwriteIfExists(parent, filePath)) {
        return false;
    }

    if (!convert()) {
        return false;
    }

    if (conversionDialog.dontAskAgain()) {
        SettingsCache::instance().visualDeckStorage().setVisualDeckStoragePromptForConversion(false);
        SettingsCache::instance().visualDeckStorage().setVisualDeckStorageAlwaysConvert(true);
    }

    return true;
}
