/**
 * @file deck_preview_tag_addition_widget.h
 * @ingroup VisualDeckPreviewWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_PREVIEW_TAG_ADDITION_WIDGET_H
#define DECK_PREVIEW_TAG_ADDITION_WIDGET_H

#include "deck_preview_deck_tags_display_widget.h"

class DeckPreviewTagAdditionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewTagAdditionWidget(QWidget *_parent, QString _tagName);
    [[nodiscard]] QSize sizeHint() const override;

signals:
    void tagClicked(); // Emitted when the tag is clicked
    void tagClosed();  // Emitted when the close button is clicked

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QString tagName_;
};

#endif // DECK_PREVIEW_TAG_ADDITION_WIDGET_H
