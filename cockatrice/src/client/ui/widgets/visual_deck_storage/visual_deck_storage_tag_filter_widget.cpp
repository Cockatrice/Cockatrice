#include "visual_deck_storage_tag_filter_widget.h"

#include "../general/layout_containers/flow_widget.h"
#include "deck_preview/deck_preview_tag_addition_widget.h"
#include "deck_preview/deck_preview_tag_display_widget.h"
#include "deck_preview/deck_preview_widget.h"

#include <QHBoxLayout>
#include <QLabel>

VisualDeckStorageTagFilterWidget::VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent)
    : QWidget(_parent), parent(_parent)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);

    setFixedHeight(100);

    auto *flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    layout->addWidget(flowWidget);
}

QList<DeckPreviewWidget *>
VisualDeckStorageTagFilterWidget::filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const
{
    // Collect selected tags from DeckPreviewTagDisplayWidget
    QStringList selectedTags;
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        if (tagWidget->getSelected()) {
            selectedTags.append(tagWidget->getTagName());
        }
    }

    // If no tags are selected, return all decks
    if (selectedTags.isEmpty()) {
        return deckPreviews;
    }

    // Filter DeckPreviewWidgets that contain all of the selected tags
    QList<DeckPreviewWidget *> filteredDecks;
    for (DeckPreviewWidget *deckPreview : deckPreviews) {
        QStringList deckTags = deckPreview->deckLoader->getTags();

        // Check if all selectedTags are in deckTags
        bool allTagsPresent = std::all_of(selectedTags.begin(), selectedTags.end(),
                                          [&deckTags](const QString &tag) { return deckTags.contains(tag); });

        if (allTagsPresent) {
            filteredDecks.append(deckPreview);
        }
    }

    return filteredDecks;
}

void VisualDeckStorageTagFilterWidget::removeTagsNotInList(const QStringList &tags)
{
    // Iterate through all DeckPreviewTagDisplayWidgets
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        // If the tag is not in the provided tags list, remove the widget
        if (!tags.contains(tagWidget->getTagName())) {
            auto *flowWidget = findChild<FlowWidget *>();
            flowWidget->removeWidget(tagWidget);
            tagWidget->deleteLater(); // Safely delete the widget
        }
    }
}

void VisualDeckStorageTagFilterWidget::addTagsIfNotPresent(const QStringList &tags)
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
                &VisualDeckStorageWidget::refreshBannerCards);
        auto *flowWidget = findChild<FlowWidget *>();
        flowWidget->addWidget(newTagWidget);
    }
}
