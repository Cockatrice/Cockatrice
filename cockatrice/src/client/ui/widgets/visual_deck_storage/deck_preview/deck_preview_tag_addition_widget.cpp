#include "deck_preview_tag_addition_widget.h"

#include "deck_preview_tag_dialog.h"

#include <QHBoxLayout>
#include <QMouseEvent>

DeckPreviewTagAdditionWidget::DeckPreviewTagAdditionWidget(DeckPreviewDeckTagsDisplayWidget *_parent,
                                                           const QString &tagName)
    : QWidget(_parent), parent(_parent), tagName_(tagName)
{
    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Create label for the tag name
    tagLabel_ = new QLabel(tagName_, this);
    layout->addWidget(tagLabel_);

    // Create close button
    closeButton_ = new QPushButton("x", this);
    closeButton_->setFixedSize(16, 16); // Small square button
    layout->addWidget(closeButton_);

    // Adjust widget size
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // Connect close button to the remove signal
    connect(closeButton_, &QPushButton::clicked, this, &DeckPreviewTagAdditionWidget::tagClosed);
}

void DeckPreviewTagAdditionWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit tagClicked();
    }
    QWidget::mousePressEvent(event);
    QStringList knownTags = parent->parent->parent->gatherAllTagsFromFlowWidget();
    QStringList activeTags = parent->deckLoader->getTags();

    DeckPreviewTagDialog dialog(knownTags, activeTags);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList updatedTags = dialog.getActiveTags();
        parent->deckLoader->setTags(updatedTags);
        parent->deckLoader->saveToFile(parent->parent->filePath, DeckLoader::CockatriceFormat);
        parent->parent->parent->refreshBannerCards();
    }
}
