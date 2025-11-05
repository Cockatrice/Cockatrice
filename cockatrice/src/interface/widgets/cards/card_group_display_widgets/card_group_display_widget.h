/**
 * @file card_group_display_widget.h
 * @ingroup DeckEditorCardGroupWidgets
 * @brief TODO: Document this.
 */

#ifndef CARD_GROUP_DISPLAY_WIDGET_H
#define CARD_GROUP_DISPLAY_WIDGET_H

#include "../../general/display/banner_widget.h"
#include "../card_info_picture_with_text_overlay_widget.h"
#include "../card_size_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

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
    void clearAllDisplayWidgets();

    DeckListModel *deckListModel;
    QPersistentModelIndex trackedIndex;
    QHash<QPersistentModelIndex, QWidget *> indexToWidgetMap;
    QString zoneName;
    QString cardGroupCategory;
    QString activeGroupCriteria;
    QStringList activeSortCriteria;
    CardSizeWidget *cardSizeWidget;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(const ExactCard &card);
    virtual QWidget *constructWidgetForIndex(QPersistentModelIndex index);
    virtual void updateCardDisplays();
    virtual void onCardAddition(const QModelIndex &parent, int first, int last);
    virtual void onCardRemoval(const QModelIndex &parent, int first, int last);
    void onActiveSortCriteriaChanged(QStringList activeSortCriteria);
    void resizeEvent(QResizeEvent *event) override;

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(const ExactCard &card);
    void cleanupRequested(CardGroupDisplayWidget *cardGroupDisplayWidget);

protected:
    QVBoxLayout *layout;
    BannerWidget *banner;

    virtual QWidget *getLayoutParent()
    {
        return this;
    }

    virtual void addToLayout(QWidget *toAdd)
    {
        layout->addWidget(toAdd);
    }

    virtual void insertIntoLayout(QWidget *toInsert, int insertAt)
    {
        layout->insertWidget(insertAt, toInsert);
    }

    virtual void removeFromLayout(QWidget *toRemove)
    {
        layout->removeWidget(toRemove);
    }
};
#endif // CARD_GROUP_DISPLAY_WIDGET_H
