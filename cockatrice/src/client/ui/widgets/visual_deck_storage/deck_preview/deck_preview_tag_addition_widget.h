#ifndef DECK_PREVIEW_TAG_ADDITION_WIDGET_H
#define DECK_PREVIEW_TAG_ADDITION_WIDGET_H

#include "deck_preview_deck_tags_display_widget.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class DeckPreviewTagAdditionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewTagAdditionWidget(QWidget *_parent,
                                          DeckPreviewDeckTagsDisplayWidget *_tagsDisplayWidget,
                                          const QString &_tagName);
    QSize sizeHint() const override;

signals:
    void tagClicked(); // Emitted when the tag is clicked
    void tagClosed();  // Emitted when the close button is clicked

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    DeckPreviewDeckTagsDisplayWidget *tagsDisplayWidget;
    QString tagName_;
    QLabel *tagLabel_;
    QPushButton *closeButton_;
};

#endif // DECK_PREVIEW_TAG_ADDITION_WIDGET_H
