#include "deck_preview_deck_tags_display_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../deck_loader/deck_loader.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
#include "deck_preview_tag_dialog.h"
#include "deck_preview_tag_display_widget.h"

#include <QDirIterator>
#include <QHBoxLayout>
#include <libcockatrice/settings/paths_settings.h>

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

void DeckPreviewDeckTagsDisplayWidget::setKnownTagsProvider(const std::function<QStringList()> &provider)
{
    knownTagsProvider_ = provider;
}

void DeckPreviewDeckTagsDisplayWidget::setConversionPromptHandler(const std::function<bool()> &handler)
{
    conversionPromptHandler_ = handler;
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
    QStringList allFiles = getAllFiles(SettingsCache::instance().paths().getDeckPath());

    QStringList knownTags;
    for (const QString &file : allFiles) {
        std::optional<LoadedDeck> deckOpt =
            DeckLoader::loadFromFile(file, DeckFileFormat::getFormatFromName(file), false);
        QStringList tags = deckOpt.has_value() ? deckOpt->deckList.getTags() : QStringList();
        knownTags.append(tags);
        knownTags.removeDuplicates();
    }

    return knownTags;
}

void DeckPreviewDeckTagsDisplayWidget::openTagEditDlg()
{
    // The deck editor path has no conversion prompt; the VDS path registers one.
    if (conversionPromptHandler_ && !conversionPromptHandler_()) {
        return;
    }

    const QStringList knownTags = knownTagsProvider_ ? knownTagsProvider_() : findAllKnownTags();
    execTagDialog(knownTags);
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
