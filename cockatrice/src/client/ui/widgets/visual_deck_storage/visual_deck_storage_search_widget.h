#ifndef VISUAL_DECK_STORAGE_SEARCH_WIDGET_H
#define VISUAL_DECK_STORAGE_SEARCH_WIDGET_H

#include "visual_deck_storage_widget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QTimer>
#include <QWidget>

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
