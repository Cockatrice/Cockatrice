#ifndef DECK_CARD_ZONE_DISPLAY_WIDGET_H
#define DECK_CARD_ZONE_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_info.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/overlap_widget.h"
#include "../visual_deck_editor/visual_deck_editor_widget.h"
#include "card_group_display_widgets/card_group_display_widget.h"
#include "card_info_picture_with_text_overlay_widget.h"
#include "card_size_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class DeckCardZoneDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DeckCardZoneDisplayWidget(QWidget *parent,
                              DeckListModel *deckListModel,
                              QPersistentModelIndex trackedIndex,
                              QString zoneName,
                              QString activeGroupCriteria,
                              QStringList activeSortCriteria,
                              DisplayType displayType,
                              int bannerOpacity,
                              int subBannerOpacity,
                              CardSizeWidget *_cardSizeWidget);
    DeckListModel *deckListModel;
    QPersistentModelIndex trackedIndex;
    QString zoneName;
    void addCardsToOverlapWidget();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(const ExactCard &card);
    void cleanupInvalidCardGroup(CardGroupDisplayWidget *displayWidget);
    void constructAppropriateWidget(QPersistentModelIndex index);
    void displayCards();
    void refreshDisplayType(const DisplayType &displayType);
    void onActiveGroupCriteriaChanged(QString activeGroupCriteria);
    void onActiveSortCriteriaChanged(QStringList activeSortCriteria);
    QList<QString> getGroupCriteriaValueList();
    void onCategoryAddition(const QModelIndex &parent, int first, int last);
    void onCategoryRemoval(const QModelIndex &parent, int first, int last);

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card, QString zoneName);
    void cardHovered(const ExactCard &card);
    void activeSortCriteriaChanged(QStringList activeSortCriteria);
    void requestCleanup(DeckCardZoneDisplayWidget *displayWidget);

private:
    QString activeGroupCriteria;
    QStringList activeSortCriteria;
    DisplayType displayType = DisplayType::Overlap;
    int bannerOpacity = 20;
    int subBannerOpacity = 10;
    CardSizeWidget *cardSizeWidget;
    QVBoxLayout *layout;
    BannerWidget *banner;
    QWidget *cardGroupContainer;
    QVBoxLayout *cardGroupLayout;
    OverlapWidget *overlapWidget;
    QHash<QPersistentModelIndex, QWidget *> indexToWidgetMap;
};

#endif // DECK_CARD_ZONE_DISPLAY_WIDGET_H
