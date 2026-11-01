#include "deck_preview_deck_tags_display_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../deck_loader/deck_loader.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
#include "deck_preview_tag_dialog.h"
#include "deck_preview_tag_display_widget.h"
#include "libcockatrice/deck_list/deck_list.h"

#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QHBoxLayout>
#include <QPointer>
#include <QSizePolicy>
#include <libcockatrice/deck_list/deck_file_format.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <optional>
#include <qnamespace.h>

DeckPreviewDeckTagsDisplayWidget::DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, const QStringList &_tags)
    : QWidget(_parent), currentTags(_tags)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = QtUtils::makeLayout<QHBoxLayout>(this);

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
    knownTagsProvider = provider;
}

void DeckPreviewDeckTagsDisplayWidget::setConversionPromptHandler(const std::function<bool()> &handler)
{
    conversionPromptHandler = handler;
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
    const QPointer<DeckPreviewDeckTagsDisplayWidget> self(this);
    // Copy callbacks before invoking them: a nested event loop can destroy this widget
    // and its stored callbacks while the callback is still running.
    const auto prompt = conversionPromptHandler;
    // The deck editor path has no conversion prompt; the VDS path registers one.
    if (prompt && !prompt()) {
        return;
    }
    if (!self) {
        return;
    }

    const auto tagsProvider = knownTagsProvider;
    const QStringList knownTags = tagsProvider ? tagsProvider() : findAllKnownTags();
    if (!self) {
        return;
    }
    execTagDialog(knownTags);
}

void DeckPreviewDeckTagsDisplayWidget::execTagDialog(const QStringList &knownTags)
{
    QPointer<DeckPreviewDeckTagsDisplayWidget> self(this);
    DeckPreviewTagDialog dialog(knownTags, currentTags);
    if (dialog.exec() == QDialog::Accepted) {
        // Guard against the widget being destroyed during dialog.exec()'s nested event loop.
        if (!self) {
            return;
        }
        QStringList updatedTags = dialog.getActiveTags();
        if (updatedTags != currentTags) {
            setTags(updatedTags);
            emit tagsChanged(updatedTags);
        }
    }
}
