#ifndef VISUAL_DECK_STORAGE_SORT_WIDGET_H
#define VISUAL_DECK_STORAGE_SORT_WIDGET_H

#include "visual_deck_storage_widget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageSortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent);
    void updateSortOrder();
    QString getSearchText();
    QStringList filterFiles(QStringList &files);

public slots:
    void showEvent(QShowEvent *event) override;

signals:
    void sortOrderChanged();

private:
    enum SortOrder
    {
        Alphabetical,
        ByLastModified
    };
    QHBoxLayout *layout;
    VisualDeckStorageWidget *parent;
    SortOrder sortOrder; // Current sorting option
    QComboBox *sortComboBox;
};

#endif // VISUAL_DECK_STORAGE_SORT_WIDGET_H
