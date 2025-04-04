#ifndef VISUAL_DECK_STORAGE_SORT_WIDGET_H
#define VISUAL_DECK_STORAGE_SORT_WIDGET_H

#include "visual_deck_storage_widget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageFolderDisplayWidget;
class VisualDeckStorageSortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();
    void updateSortOrder();
    std::function<bool(DeckPreviewWidget *, DeckPreviewWidget *)> getComparator() const;

    QString getSearchText();

signals:
    void sortOrderChanged();

private:
    enum SortOrder
    {
        ByName,
        Alphabetical,
        ByLastModified,
        ByLastLoaded,
    };
    QHBoxLayout *layout;
    VisualDeckStorageWidget *parent;
    SortOrder sortOrder; // Current sorting option
    QComboBox *sortComboBox;
};

#endif // VISUAL_DECK_STORAGE_SORT_WIDGET_H
