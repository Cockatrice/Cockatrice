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
                               QPersistentModelIndex trackedIndex,
                               QString zoneName,
                               QString cardGroupCategory,
                               QString activeGroupCriteria,
                               QStringList activeSortCriteria,
                               int bannerOpacity,
                               CardSizeWidget *cardSizeWidget);

public slots:
    QWidget *constructWidgetForIndex(int row) override;
    void resizeEvent(QResizeEvent *event) override;
    void onCardAddition(const QModelIndex &parent, int first, int last) override;
    void onCardRemoval(const QModelIndex &parent, int first, int last) override;

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
