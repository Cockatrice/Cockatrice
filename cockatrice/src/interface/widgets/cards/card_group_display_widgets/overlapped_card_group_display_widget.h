/**
 * @file overlapped_card_group_display_widget.h
 * @ingroup DeckEditorCardGroupWidgets
 * @brief TODO: Document this.
 */

#ifndef OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
#define OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../general/layout_containers/overlap_widget.h"
#include "card_group_display_widget.h"

class OverlappedCardGroupDisplayWidget : public CardGroupDisplayWidget
{
    Q_OBJECT

public:
    OverlappedCardGroupDisplayWidget(QWidget *parent,
                                     DeckListModel *deckListModel,
                                     QItemSelectionModel *selectionModel,
                                     QPersistentModelIndex trackedIndex,
                                     QString zoneName,
                                     QString cardGroupCategory,
                                     QString activeGroupCriteria,
                                     QStringList activeSortCriteria,
                                     int bannerOpacity,
                                     CardSizeWidget *cardSizeWidget);

public slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    OverlapWidget *overlapWidget;

    QWidget *getLayoutParent() override
    {
        return overlapWidget;
    }

    void addToLayout(QWidget *toAdd) override
    {
        overlapWidget->addWidget(toAdd);
    }

    void insertIntoLayout(QWidget *toInsert, int insertAt) override
    {
        overlapWidget->insertWidgetAtIndex(toInsert, insertAt);
    }

    void removeFromLayout(QWidget *toRemove) override
    {
        overlapWidget->removeWidget(toRemove);
    }
};

#endif // OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
