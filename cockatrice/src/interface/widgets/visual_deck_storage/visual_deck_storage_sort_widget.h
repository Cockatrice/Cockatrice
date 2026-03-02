/**
 * @file visual_deck_storage_sort_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

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
    void sortFolder(VisualDeckStorageFolderDisplayWidget *folderWidget);
    QString getSearchText();
    QList<DeckPreviewWidget *> filterFiles(QList<DeckPreviewWidget *> widgets);

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
