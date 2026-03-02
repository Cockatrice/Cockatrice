/**
 * @file flat_card_group_display_widget.h
 * @ingroup DeckEditorCardGroupWidgets
 * @brief TODO: Document this.
 */

#ifndef FLAT_CARD_GROUP_DISPLAY_WIDGET_H
#define FLAT_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../general/layout_containers/flow_widget.h"
#include "card_group_display_widget.h"

class FlatCardGroupDisplayWidget : public CardGroupDisplayWidget
{
    Q_OBJECT

public:
    FlatCardGroupDisplayWidget(QWidget *parent,
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
    FlowWidget *flowWidget;

    QWidget *getLayoutParent() override
    {
        return flowWidget;
    }

    void addToLayout(QWidget *toAdd) override
    {
        flowWidget->addWidget(toAdd);
    }

    void insertIntoLayout(QWidget *toInsert, int insertAt) override
    {
        flowWidget->insertWidgetAtIndex(toInsert, insertAt);
    }

    void removeFromLayout(QWidget *toRemove) override
    {
        flowWidget->removeWidget(toRemove);
    }
};

#endif // FLAT_CARD_GROUP_DISPLAY_WIDGET_H
