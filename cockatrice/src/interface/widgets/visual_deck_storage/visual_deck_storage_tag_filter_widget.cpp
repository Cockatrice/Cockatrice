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

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    layout->addWidget(flowWidget);
}

void VisualDeckStorageTagFilterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshTags();
}

/**
 * @brief The tags of all decks currently accepted by the proxy model.
 */
QSet<QString> VisualDeckStorageTagFilterWidget::gatherAllTags() const
{
    QSet<QString> allTags;
    auto *proxy = parent->proxyModel();
    auto *model = parent->model();

    for (int proxyRow = 0; proxyRow < proxy->rowCount(); ++proxyRow) {
        const QModelIndex sourceIndex = proxy->mapToSource(proxy->index(proxyRow, 0));
        if (!sourceIndex.isValid()) {
            continue;
        }
        for (const QString &tag : model->dataForRow(sourceIndex.row()).tags) {
            allTags.insert(tag);
        }
    }

    return allTags;
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
        flowWidget->addWidget(newTagWidget);
    }
}

void VisualDeckStorageTagFilterWidget::sortTags()
{
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

QStringList VisualDeckStorageTagFilterWidget::selectedTags() const
{
    QStringList selected;
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        if (tagWidget->getState() == TagState::Selected) {
            selected.append(tagWidget->getTagName());
        }
    }
    return selected;
}

QStringList VisualDeckStorageTagFilterWidget::excludedTags() const
{
    QStringList excluded;
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        if (tagWidget->getState() == TagState::Excluded) {
            excluded.append(tagWidget->getTagName());
        }
    }
    return excluded;
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
