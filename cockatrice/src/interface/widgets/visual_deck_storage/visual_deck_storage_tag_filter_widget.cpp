#include "visual_deck_storage_tag_filter_widget.h"

#include "../general/layout_containers/flow_widget.h"
#include "deck_preview/deck_preview_tag_display_widget.h"
#include "visual_deck_storage_widget.h"

#include <QHBoxLayout>

VisualDeckStorageTagFilterWidget::VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent)
    : QWidget(_parent), parent(_parent)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);

    setFixedHeight(100);

    auto *flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    layout->addWidget(flowWidget);
}

void VisualDeckStorageTagFilterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshTags();
}

void VisualDeckStorageTagFilterWidget::filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const
{
    QStringList selectedTags;
    QStringList excludedTags;

    // Collect selected and excluded tags
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        switch (tagWidget->getState()) {
            case TagState::Selected:
                selectedTags.append(tagWidget->getTagName());
                break;
            case TagState::Excluded:
                excludedTags.append(tagWidget->getTagName());
                break;
            default:
                break;
        }
    }

    // If no tags are selected or excluded, show all
    if (selectedTags.isEmpty() && excludedTags.isEmpty()) {
        for (DeckPreviewWidget *deckPreview : deckPreviews) {
            deckPreview->filteredByTags = false;
        }
        return;
    }

    for (DeckPreviewWidget *deckPreview : deckPreviews) {
        QStringList deckTags = deckPreview->deckLoader->getDeck().deckList.getTags();

        bool hasAllSelected = std::all_of(selectedTags.begin(), selectedTags.end(),
                                          [&deckTags](const QString &tag) { return deckTags.contains(tag); });

        bool hasAnyExcluded = std::any_of(excludedTags.begin(), excludedTags.end(),
                                          [&deckTags](const QString &tag) { return deckTags.contains(tag); });

        // Filter out if any excluded tag is present or if any selected tag is missing
        deckPreview->filteredByTags = !(hasAllSelected && !hasAnyExcluded);
    }
}

void VisualDeckStorageTagFilterWidget::refreshTags()
{
    QSet<QString> allTags = gatherAllTags();
    removeTagsNotInList(allTags);
    addTagsIfNotPresent(allTags);
    sortTags();
}

void VisualDeckStorageTagFilterWidget::removeTagsNotInList(const QSet<QString> &tags)
{
    auto *flowWidget = findChild<FlowWidget *>();

    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        const QString &tagName = tagWidget->getTagName();

        // Keep the tag widget if it is either selected or excluded
        if (!tags.contains(tagName) && tagWidget->getState() == TagState::NotSelected) {
            flowWidget->removeWidget(tagWidget);
            tagWidget->deleteLater();
        }
    }
}

void VisualDeckStorageTagFilterWidget::addTagsIfNotPresent(const QSet<QString> &tags)
{
    for (const QString &tag : tags) {
        addTagIfNotPresent(tag);
    }
}

void VisualDeckStorageTagFilterWidget::addTagIfNotPresent(const QString &tag)
{
    // Check if the tag already exists in the flow widget
    bool tagExists = false;
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        if (tagWidget->getTagName() == tag) {
            tagExists = true;
            break;
        }
    }

    // If the tag doesn't exist, add a new DeckPreviewTagDisplayWidget
    if (!tagExists) {
        auto *newTagWidget = new DeckPreviewTagDisplayWidget(this, tag);
        connect(newTagWidget, &DeckPreviewTagDisplayWidget::tagClicked, parent,
                &VisualDeckStorageWidget::updateTagFilter);
        connect(newTagWidget, &DeckPreviewTagDisplayWidget::tagClicked, this,
                &VisualDeckStorageTagFilterWidget::refreshTags);
        auto *flowWidget = findChild<FlowWidget *>();
        flowWidget->addWidget(newTagWidget);
    }
}

void VisualDeckStorageTagFilterWidget::sortTags()
{
    auto *flowWidget = findChild<FlowWidget *>();
    if (!flowWidget)
        return;

    // Get all tag widgets
    QList<DeckPreviewTagDisplayWidget *> tagWidgets = findChildren<DeckPreviewTagDisplayWidget *>();

    // Sort widgets by tag name
    std::sort(tagWidgets.begin(), tagWidgets.end(), [](DeckPreviewTagDisplayWidget *a, DeckPreviewTagDisplayWidget *b) {
        return a->getTagName().toLower() < b->getTagName().toLower();
    });

    // Clear and re-add widgets in sorted order
    for (DeckPreviewTagDisplayWidget *tagWidget : tagWidgets) {
        flowWidget->removeWidget(tagWidget);
    }
    for (DeckPreviewTagDisplayWidget *tagWidget : tagWidgets) {
        flowWidget->addWidget(tagWidget);
    }
}

QSet<QString> VisualDeckStorageTagFilterWidget::gatherAllTags() const
{
    QSet<QString> allTags;
    QList<DeckPreviewWidget *> deckWidgets = parent->findChildren<DeckPreviewWidget *>();

    for (DeckPreviewWidget *widget : deckWidgets) {
        if (widget->checkVisibility()) {
            for (const QString &tag : widget->deckLoader->getDeck().deckList.getTags()) {
                allTags.insert(tag);
            }
        }
    }
    return allTags;
}

QStringList VisualDeckStorageTagFilterWidget::getAllKnownTags() const
{
    QStringList allTags;

    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        allTags.append(tagWidget->getTagName());
    }

    // Remove duplicates by calling 'removeDuplicates'
    allTags.removeDuplicates();

    return allTags;
}
