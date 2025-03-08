#ifndef DECK_CARD_ZONE_DISPLAY_WIDGET_H
#define DECK_CARD_ZONE_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/overlap_widget.h"
#include "card_info_picture_with_text_overlay_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class DeckCardZoneDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DeckCardZoneDisplayWidget(QWidget *parent,
                              DeckListModel *deckListModel,
                              QString zoneName,
                              QString activeGroupCriteria,
                              QStringList activeSortCriteria,
                              int bannerOpacity,
                              int subBannerOpacity);
    DeckListModel *deckListModel;
    QString zoneName;
    void addCardsToOverlapWidget();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);
    void displayCards();
    void addCardGroupIfItDoesNotExist();
    void deleteCardGroupIfItDoesNotExist();
    void onActiveGroupCriteriaChanged(QString activeGroupCriteria);
    void onActiveSortCriteriaChanged(QStringList activeSortCriteria);
    QList<QString> getGroupCriteriaValueList();

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card, QString zoneName);
    void cardHovered(CardInfoPtr card);
    void activeSortCriteriaChanged(QStringList activeSortCriteria);

private:
    QString activeGroupCriteria;
    QStringList activeSortCriteria;
    QVBoxLayout *layout;
    BannerWidget *banner;
    QWidget *cardGroupContainer;
    QVBoxLayout *cardGroupLayout;
    int bannerOpacity = 20;
    int subBannerOpacity = 10;
    OverlapWidget *overlapWidget;
};

#endif // DECK_CARD_ZONE_DISPLAY_WIDGET_H
