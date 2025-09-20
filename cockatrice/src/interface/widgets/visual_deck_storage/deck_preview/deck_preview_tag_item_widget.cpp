#include "deck_preview_tag_item_widget.h"

DeckPreviewTagItemWidget::DeckPreviewTagItemWidget(const QString &tagName, bool isChecked, QWidget *parent)
    : QWidget(parent), checkBox_(new QCheckBox(this))
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    checkBox_->setText(tagName);      // Set the tag name as the checkbox label
    checkBox_->setChecked(isChecked); // Set the initial state of the checkbox

    layout->addWidget(checkBox_); // Add the checkbox to the layout
    setLayout(layout);            // Set the layout of this widget
}

QCheckBox *DeckPreviewTagItemWidget::checkBox() const
{
    return checkBox_; // Return the checkbox widget
}
