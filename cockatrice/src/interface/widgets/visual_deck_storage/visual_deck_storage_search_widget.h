/**
 * @file visual_deck_storage_search_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
#define VISUAL_DECK_STORAGE_SEARCH_WIDGET_H

#include <QWidget>

class DeckPreviewWidget;
class QHBoxLayout;
class QLineEdit;
class VisualDeckStorageWidget;
class VisualDeckStorageSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSearchWidget(VisualDeckStorageWidget *parent);
    QString getSearchText();
    void filterWidgets(QList<DeckPreviewWidget *> widgets, const QString &searchText);

private:
    QHBoxLayout *layout;
    VisualDeckStorageWidget *parent;
    QLineEdit *searchBar;
    QTimer *searchDebounceTimer;
};

#endif // VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
