#include "deck_preview_deck_tags_display_widget.h"

#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_addition_widget.h"
#include "deck_preview_tag_display_widget.h"
#include "deck_preview_widget.h"

#include <QHBoxLayout>
#include <QLabel>

DeckPreviewDeckTagsDisplayWidget::DeckPreviewDeckTagsDisplayWidget(DeckPreviewWidget *_parent, DeckLoader *_deckLoader)
    : QWidget(_parent), parent(_parent), deckLoader(_deckLoader)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    setFixedHeight(100);

    auto *flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    for (const QString &tag : this->deckLoader->getTags()) {
        flowWidget->addWidget(new DeckPreviewTagDisplayWidget(this, tag));
    }
    flowWidget->addWidget(new DeckPreviewTagAdditionWidget(this, tr("Edit tags ...")));
    layout->addWidget(flowWidget);
}