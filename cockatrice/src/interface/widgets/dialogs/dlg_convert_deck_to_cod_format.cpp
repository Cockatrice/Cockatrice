#include "dlg_convert_deck_to_cod_format.h"

#include "../../../client/settings/cache_settings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QVBoxLayout>
#include <libcockatrice/deck_list/deck_file_format.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/qt_utils.h>

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

namespace
{

bool confirmOverwriteIfExists(QWidget *parent, const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString newFileName = QDir::toNativeSeparators(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".cod");

    if (QFile::exists(newFileName)) {
        return QtUtils::confirm(parent, QObject::tr("Overwrite Existing File?"),
                                QObject::tr("A .cod version of this deck already exists. Overwrite it?"));
    }
    return true; // Safe to proceed
}

} // namespace

bool DialogConvertDeckToCodFormat::promptIfRequired(QWidget *parent,
                                                    const QString &filePath,
                                                    const std::function<bool()> &convert)
{
    // The caller can be destroyed while a prompt runs its nested event loop.
    // Keep inputs alive independently, and distinguish a null parent from a deleted one.
    const QString deckFilePath = filePath;
    const std::function<bool()> convertDeck = convert;
    const bool hadParent = parent != nullptr;
    const QPointer<QWidget> parentGuard(parent);
    const auto parentWasDeleted = [&] { return hadParent && parentGuard.isNull(); };

    if (DeckFileFormat::getFormatFromName(deckFilePath) == DeckFileFormat::Cockatrice) {
        return true;
    }

    // Retrieve saved preference if the prompt is disabled
    if (!SettingsCache::instance().visualDeckStorage().getVisualDeckStoragePromptForConversion()) {
        if (!SettingsCache::instance().visualDeckStorage().getVisualDeckStorageAlwaysConvert()) {
            return false;
        }

        if (!confirmOverwriteIfExists(parentGuard.data(), deckFilePath) || parentWasDeleted()) {
            return false;
        }

        const bool converted = convertDeck();
        return converted && !parentWasDeleted();
    }

    // Parent destruction may delete the dialog during exec(), so it must not live on the stack.
    QPointer<DialogConvertDeckToCodFormat> conversionDialog = new DialogConvertDeckToCodFormat(parentGuard.data());
    const int result = conversionDialog->exec();
    if (!conversionDialog) {
        return false;
    }
    const bool rememberChoice = conversionDialog->dontAskAgain();
    delete conversionDialog.data();
    if (parentWasDeleted()) {
        return false;
    }

    if (result != QDialog::Accepted) {
        SettingsCache::instance().visualDeckStorage().setVisualDeckStoragePromptForConversion(!rememberChoice);
        SettingsCache::instance().visualDeckStorage().setVisualDeckStorageAlwaysConvert(false);

        return false;
    }

    // Try to convert file
    if (!confirmOverwriteIfExists(parentGuard.data(), deckFilePath) || parentWasDeleted()) {
        return false;
    }

    if (!convertDeck() || parentWasDeleted()) {
        return false;
    }

    if (rememberChoice) {
        SettingsCache::instance().visualDeckStorage().setVisualDeckStoragePromptForConversion(false);
        SettingsCache::instance().visualDeckStorage().setVisualDeckStorageAlwaysConvert(true);
    }

    return true;
}
