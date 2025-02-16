#include "deck_preview_deck_tags_display_widget.h"

#include "../../../../../dialogs/dlg_convert_deck_to_cod_format.h"
#include "../../../../../settings/cache_settings.h"
#include "../../../../tabs/tab_deck_editor.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
#include "deck_preview_tag_dialog.h"
#include "deck_preview_tag_display_widget.h"
#include "deck_preview_widget.h"

#include <QHBoxLayout>
#include <QLabel>

DeckPreviewDeckTagsDisplayWidget::DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, DeckList *_deckList)
    : QWidget(_parent), deckList(_deckList)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    setFixedHeight(100);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    if (deckList) {
        connectDeckList(deckList);
    }

    layout->addWidget(flowWidget);
}

void DeckPreviewDeckTagsDisplayWidget::connectDeckList(DeckList *_deckList)
{
    flowWidget->clearLayout();
    deckList = _deckList;
    connect(deckList, &DeckList::deckTagsChanged, this, &DeckPreviewDeckTagsDisplayWidget::refreshTags);

    for (const QString &tag : deckList->getTags()) {
        flowWidget->addWidget(new DeckPreviewTagDisplayWidget(this, tag));
    }

    auto tagAdditionWidget = new DeckPreviewTagAdditionWidget(this, tr("Edit tags ..."));
    connect(tagAdditionWidget, &DeckPreviewTagAdditionWidget::tagClicked, this,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);
    flowWidget->addWidget(tagAdditionWidget);
}

void DeckPreviewDeckTagsDisplayWidget::refreshTags()
{
    flowWidget->clearLayout();
    QStringList tags = deckList->getTags();
    for (const QString &tag : tags) {
        flowWidget->addWidget(new DeckPreviewTagDisplayWidget(this, tag));
    }
    flowWidget->addWidget(new DeckPreviewTagAdditionWidget(this, tr("Edit tags ...")));
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

void DeckPreviewDeckTagsDisplayWidget::openTagEditDlg()
{
    if (qobject_cast<DeckPreviewWidget *>(parentWidget())) {
        auto *deckPreviewWidget = qobject_cast<DeckPreviewWidget *>(parentWidget());
        QStringList knownTags = deckPreviewWidget->visualDeckStorageWidget->tagFilterWidget->getAllKnownTags();
        QStringList activeTags = deckList->getTags();

        bool canAddTags = true;

        if (DeckLoader::getFormatFromName(deckPreviewWidget->filePath) != DeckLoader::CockatriceFormat) {
            canAddTags = false;
            // Retrieve saved preference if the prompt is disabled
            if (!SettingsCache::instance().getVisualDeckStoragePromptForConversion()) {
                if (SettingsCache::instance().getVisualDeckStorageAlwaysConvert()) {
                    deckPreviewWidget->deckLoader->convertToCockatriceFormat(deckPreviewWidget->filePath);
                    deckPreviewWidget->filePath = deckPreviewWidget->deckLoader->getLastFileName();
                    deckPreviewWidget->refreshBannerCardText();
                    canAddTags = true;
                }
            } else {
                // Show the dialog to the user
                DialogConvertDeckToCodFormat conversionDialog(parentWidget());
                if (conversionDialog.exec() == QDialog::Accepted) {
                    deckPreviewWidget->deckLoader->convertToCockatriceFormat(deckPreviewWidget->filePath);
                    deckPreviewWidget->filePath = deckPreviewWidget->deckLoader->getLastFileName();
                    deckPreviewWidget->refreshBannerCardText();
                    canAddTags = true;

                    if (conversionDialog.dontAskAgain()) {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Unchecked);
                        SettingsCache::instance().setVisualDeckStorageAlwaysConvert(Qt::CheckState::Checked);
                    }
                } else {
                    SettingsCache::instance().setVisualDeckStorageAlwaysConvert(Qt::CheckState::Unchecked);

                    if (conversionDialog.dontAskAgain()) {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Unchecked);
                    } else {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Checked);
                    }
                }
            }
        }

        if (canAddTags) {
            DeckPreviewTagDialog dialog(knownTags, activeTags);
            if (dialog.exec() == QDialog::Accepted) {
                QStringList updatedTags = dialog.getActiveTags();
                deckList->setTags(updatedTags);
                deckPreviewWidget->deckLoader->saveToFile(deckPreviewWidget->filePath, DeckLoader::CockatriceFormat);
            }
        }
    } else if (parentWidget()) {
        // If we're the child of a TabDeckEditor, we are buried under a ton of childWidgets in the DeckInfoDock.
        QWidget *currentParent = parentWidget();
        while (currentParent) {
            if (qobject_cast<TabDeckEditor *>(currentParent)) {
                break;
            }
            currentParent = currentParent->parentWidget();
        }
        if (qobject_cast<TabDeckEditor *>(currentParent)) {
            auto *deckEditor = qobject_cast<TabDeckEditor *>(currentParent);
            QStringList knownTags;
            QStringList allFiles = getAllFiles(SettingsCache::instance().getDeckPath());
            auto *loader = new DeckLoader();
            for (const QString &file : allFiles) {
                loader->loadFromFile(file, DeckLoader::getFormatFromName(file), false);
                QStringList tags = loader->getTags();
                knownTags.append(tags);
                knownTags.removeDuplicates();
            }

            QStringList activeTags = deckList->getTags();

            DeckPreviewTagDialog dialog(knownTags, activeTags);
            if (dialog.exec() == QDialog::Accepted) {
                QStringList updatedTags = dialog.getActiveTags();
                deckList->setTags(updatedTags);
                deckEditor->setModified(true);
            }
        }
    }
}