/**
 * @file visual_deck_storage_search_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
#define VISUAL_DECK_STORAGE_SEARCH_WIDGET_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QWidget>

class QTimer;
class VisualDeckStorageWidget;
class VisualDeckStorageSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSearchWidget(VisualDeckStorageWidget *parent);

private:
    QHBoxLayout *layout;
    VisualDeckStorageWidget *parent;
    QLineEdit *searchBar;
    QTimer *searchDebounceTimer;
};

#endif // VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
