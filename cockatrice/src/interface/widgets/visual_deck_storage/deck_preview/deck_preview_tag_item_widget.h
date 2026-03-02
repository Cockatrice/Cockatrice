/**
 * @file deck_preview_tag_item_widget.h
 * @ingroup VisualDeckPreviewWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_PREVIEW_TAG_ITEM_WIDGET_H
#define DECK_PREVIEW_TAG_ITEM_WIDGET_H

#include <QCheckBox>
#include <QHBoxLayout>
#include <QWidget>

class DeckPreviewTagItemWidget : public QWidget
{
    Q_OBJECT
public:
    // Constructor: Initializes the tag item widget with a tag name and initial checkbox state
    DeckPreviewTagItemWidget(const QString &tagName, bool isChecked, QWidget *parent = nullptr);

    // Accessor for the checkbox widget
    [[nodiscard]] QCheckBox *checkBox() const;

private:
    QCheckBox *checkBox_; // Checkbox to represent the tag's state
};

#endif // DECK_PREVIEW_TAG_ITEM_WIDGET_H
