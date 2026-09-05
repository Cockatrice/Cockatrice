#include "visual_deck_storage_tag_filter_widget.h"

#include "../general/layout_containers/flow_widget.h"
#include "deck_preview/deck_preview_tag_display_widget.h"

#include <QHBoxLayout>

VisualDeckStorageTagFilterWidget::VisualDeckStorageTagFilterWidget(QWidget *parent) : QWidget(parent)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);

    setFixedHeight(100);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    layout->addWidget(flowWidget);
}

void VisualDeckStorageTagFilterWidget::setAllTagsProvider(const std::function<QSet<QString>()> &provider)
{
    allTagsProvider = provider;
}

void VisualDeckStorageTagFilterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshTags();
}

void VisualDeckStorageTagFilterWidget::refreshTags()
{
    const QSet<QString> allTags = allTagsProvider ? allTagsProvider() : QSet<QString>();

    // Existing chips survive if their tag is still part of the deck set, or if the chip
    // is currently selected/excluded. Everything else is dropped. Dropped chips must NOT
    // be re-added to the layout afterwards: they are scheduled for a deferred delete, and
    // the flow layout would keep a dangling reference to them once the deletion runs on
    // the next event-loop cycle.
    QList<DeckPreviewTagDisplayWidget *> chips;
    for (DeckPreviewTagDisplayWidget *tagWidget : findChildren<DeckPreviewTagDisplayWidget *>()) {
        if (tagWidget->getState() != TagState::NotSelected || allTags.contains(tagWidget->getTagName())) {
            chips.append(tagWidget);
        } else {
            flowWidget->removeWidget(tagWidget);
            tagWidget->deleteLater();
        }
    }

    // Add chips for tags that are not shown yet.
    for (const QString &tag : allTags) {
        bool tagExists = false;
        for (DeckPreviewTagDisplayWidget *tagWidget : chips) {
            if (tagWidget->getTagName() == tag) {
                tagExists = true;
                break;
            }
        }
        if (!tagExists) {
            auto *newTagWidget = new DeckPreviewTagDisplayWidget(this, tag);
            connect(newTagWidget, &DeckPreviewTagDisplayWidget::tagClicked, this,
                    &VisualDeckStorageTagFilterWidget::filterChanged);
            flowWidget->addWidget(newTagWidget);
            chips.append(newTagWidget);
        }
    }

    // Clear and re-add the chips in sorted order.
    std::sort(chips.begin(), chips.end(), [](DeckPreviewTagDisplayWidget *a, DeckPreviewTagDisplayWidget *b) {
        return a->getTagName().toLower() < b->getTagName().toLower();
    });
    for (DeckPreviewTagDisplayWidget *tagWidget : chips) {
        flowWidget->removeWidget(tagWidget);
    }
    for (DeckPreviewTagDisplayWidget *tagWidget : chips) {
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
