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

class VisualDeckStorageSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSearchWidget(QWidget *parent);

    void setPlaceholderText(const QString &text);

signals:
    /**
     * Emitted once the debounce timer fires after the user stopped typing.
     * @param text The current contents of the search bar.
     */
    void searchTextChanged(const QString &text);

private:
    QHBoxLayout *layout;
    QLineEdit *searchBar;
    QTimer *searchDebounceTimer;
};

#endif // VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
