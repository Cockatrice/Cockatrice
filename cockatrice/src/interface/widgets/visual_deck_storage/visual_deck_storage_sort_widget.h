/**
 * @file visual_deck_storage_sort_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_SORT_WIDGET_H
#define VISUAL_DECK_STORAGE_SORT_WIDGET_H

#include "visual_deck_storage_sort_filter_proxy_model.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageSortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();

    /**
     * @brief The currently selected sort order.
     */
    [[nodiscard]] VisualDeckStorageSortFilterProxyModel::SortOrder currentSortOrder() const;

signals:
    /**
     * @brief Emitted when the user picks a different sort order.
     */
    void sortOrderChanged();

private slots:
    void updateSortOrder();

private:
    QHBoxLayout *layout;
    QComboBox *sortComboBox;
};

#endif // VISUAL_DECK_STORAGE_SORT_WIDGET_H
