#ifndef CARD_GROUP_DISPLAY_WIDGET_H
#define CARD_GROUP_DISPLAY_WIDGET_H

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_info.h"
#include "../../general/display/banner_widget.h"
#include "../card_info_picture_with_text_overlay_widget.h"
#include "../card_size_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class CardGroupDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    CardGroupDisplayWidget(QWidget *parent,
                           DeckListModel *deckListModel,
                           QPersistentModelIndex trackedIndex,
                           QString zoneName,
                           QString cardGroupCategory,
                           QString activeGroupCriteria,
                           QStringList activeSortCriteria,
                           int bannerOpacity,
                           CardSizeWidget *cardSizeWidget);

    DeckListModel *deckListModel;
    QPersistentModelIndex trackedIndex;
    QHash<QPersistentModelIndex, QWidget*> indexToWidgetMap;
    QString zoneName;
    QString cardGroupCategory;
    QString activeGroupCriteria;
    QStringList activeSortCriteria;
    CardSizeWidget *cardSizeWidget;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);
    virtual QWidget* constructWidgetForIndex(int rowIndex);
    virtual void updateCardDisplays();
    void onCardAddition(const QModelIndex &parent, int first, int last);
    void onCardRemoval(const QModelIndex &parent, int first, int last);
    void resizeEvent(QResizeEvent *event) override;

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(CardInfoPtr card);

protected:
    QVBoxLayout *layout;
    BannerWidget *banner;

    virtual QWidget* getLayoutParent()
    {
        return this;
    }

    virtual void addToLayout(QWidget* toAdd)
    {
        layout->addWidget(toAdd);
    }

    virtual void insertIntoLayout(QWidget *toInsert, int insertAt)
    {
        qInfo() << "Default card group insert";
        layout->insertWidget(insertAt, toInsert);
    }

    virtual void removeFromLayout(QWidget* toRemove)
    {
        layout->removeWidget(toRemove);
    }
};
#endif // CARD_GROUP_DISPLAY_WIDGET_H
