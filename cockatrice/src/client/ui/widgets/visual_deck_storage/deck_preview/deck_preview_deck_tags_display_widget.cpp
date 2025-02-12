#include "deck_preview_deck_tags_display_widget.h"

#include "../../../../tabs/tab_deck_editor.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
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
    flowWidget->addWidget(new DeckPreviewTagAdditionWidget(this, this, tr("Edit tags ...")));
}

void DeckPreviewDeckTagsDisplayWidget::refreshTags()
{
    flowWidget->clearLayout();
    QStringList tags = deckList->getTags();
    for (const QString &tag : tags) {
        flowWidget->addWidget(new DeckPreviewTagDisplayWidget(this, tag));
    }
    flowWidget->addWidget(new DeckPreviewTagAdditionWidget(this, this, tr("Edit tags ...")));
}