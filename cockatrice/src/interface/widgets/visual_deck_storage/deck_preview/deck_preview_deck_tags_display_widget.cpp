#include "deck_preview_deck_tags_display_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../../interface/widgets/dialogs/dlg_convert_deck_to_cod_format.h"
#include "../../../../interface/widgets/tabs/tab_deck_editor.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
#include "deck_preview_tag_dialog.h"
#include "deck_preview_tag_display_widget.h"
#include "deck_preview_widget.h"

#include <QDirIterator>
#include <QHBoxLayout>
#include <QMessageBox>

DeckPreviewDeckTagsDisplayWidget::DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, const QStringList &_tags)
    : QWidget(_parent), currentTags(_tags)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    setFixedHeight(100);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    layout->addWidget(flowWidget);

    refreshTags();
}

void DeckPreviewDeckTagsDisplayWidget::setTags(const QStringList &_tags)
{
    currentTags = _tags;
    refreshTags();
}

void DeckPreviewDeckTagsDisplayWidget::refreshTags()
{
    flowWidget->clearLayout();

    for (const QString &tag : currentTags) {
        flowWidget->addWidget(new DeckPreviewTagDisplayWidget(this, tag));
    }

    auto tagAdditionWidget = new DeckPreviewTagAdditionWidget(this, tr("Edit tags ..."));
    connect(tagAdditionWidget, &DeckPreviewTagAdditionWidget::tagClicked, this,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);
    flowWidget->addWidget(tagAdditionWidget);
}

/**
 * Gets the filepath of all files (no directories) in target directory and all subdirectories
 */
static QStringList getAllFiles(const QString &filePath)
{
    QStringList allFiles;

    // QDirIterator with QDir::Files ensures only files are listed (no directories)
    QDirIterator it(filePath, QDir::Files, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    return allFiles;
}

/**
 * Gets all tags that appear in the deck folder
 */
static QStringList findAllKnownTags()
{
    QStringList allFiles = getAllFiles(SettingsCache::instance().getDeckPath());

    QStringList knownTags;
    auto loader = DeckLoader(nullptr);
    for (const QString &file : allFiles) {
        loader.loadFromFile(file, DeckFileFormat::getFormatFromName(file), false);
        QStringList tags = loader.getDeck().deckList.getTags();
        knownTags.append(tags);
        knownTags.removeDuplicates();
    }

    return knownTags;
}

void DeckPreviewDeckTagsDisplayWidget::openTagEditDlg()
{
    if (qobject_cast<DeckPreviewWidget *>(parentWidget())) {
        // If we're the child of a DeckPreviewWidget, then we need to handle conversion
        auto *deckPreviewWidget = qobject_cast<DeckPreviewWidget *>(parentWidget());

        bool canAddTags = promptFileConversionIfRequired(deckPreviewWidget);

        if (canAddTags) {
            QStringList knownTags = deckPreviewWidget->visualDeckStorageWidget->tagFilterWidget->getAllKnownTags();
            execTagDialog(knownTags);
        }
    } else {
        // If we're the child of an AbstractTabDeckEditor, then we don't bother with conversion
        QStringList knownTags = findAllKnownTags();
        execTagDialog(knownTags);
    }
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

static void convertFileToCockatriceFormat(DeckPreviewWidget *deckPreviewWidget)
{
    deckPreviewWidget->deckLoader->convertToCockatriceFormat(deckPreviewWidget->filePath);
    deckPreviewWidget->filePath = deckPreviewWidget->deckLoader->getDeck().lastLoadInfo.fileName;
    deckPreviewWidget->refreshBannerCardText();
}

/**
 * Checks if the deck's file format supports tags.
 * If not, then prompt the user for file conversion.
 * @return whether the resulting file can support adding tags
 */
bool DeckPreviewDeckTagsDisplayWidget::promptFileConversionIfRequired(DeckPreviewWidget *deckPreviewWidget)
{
    if (DeckFileFormat::getFormatFromName(deckPreviewWidget->filePath) == DeckFileFormat::Cockatrice) {
        return true;
    }

    // Retrieve saved preference if the prompt is disabled
    if (!SettingsCache::instance().getVisualDeckStoragePromptForConversion()) {
        if (!SettingsCache::instance().getVisualDeckStorageAlwaysConvert()) {
            return false;
        }

        if (!confirmOverwriteIfExists(this, deckPreviewWidget->filePath)) {
            return false;
        }

        convertFileToCockatriceFormat(deckPreviewWidget);
        return true;
    }

    // Show the dialog to the user
    DialogConvertDeckToCodFormat conversionDialog(parentWidget());
    if (conversionDialog.exec() != QDialog::Accepted) {
        SettingsCache::instance().setVisualDeckStoragePromptForConversion(!conversionDialog.dontAskAgain());
        SettingsCache::instance().setVisualDeckStorageAlwaysConvert(false);

        return false;
    }

    // Try to convert file
    if (!confirmOverwriteIfExists(this, deckPreviewWidget->filePath)) {
        return false;
    }

    convertFileToCockatriceFormat(deckPreviewWidget);

    if (conversionDialog.dontAskAgain()) {
        SettingsCache::instance().setVisualDeckStoragePromptForConversion(false);
        SettingsCache::instance().setVisualDeckStorageAlwaysConvert(true);
    }

    return true;
}

void DeckPreviewDeckTagsDisplayWidget::execTagDialog(const QStringList &knownTags)
{
    DeckPreviewTagDialog dialog(knownTags, currentTags);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList updatedTags = dialog.getActiveTags();
        if (updatedTags != currentTags) {
            setTags(updatedTags);
            emit tagsChanged(updatedTags);
        }
    }
}