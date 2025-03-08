#ifndef OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
#define OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/overlap_widget.h"
#include "card_info_picture_with_text_overlay_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class OverlappedCardGroupDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    OverlappedCardGroupDisplayWidget(QWidget *parent,
                                     DeckListModel *deckListModel,
                                     QString zoneName,
                                     QString cardGroupCategory,
                                     QString activeGroupCriteria,
                                     QStringList activeSortCriteria,
                                     int bannerOpacity);

    QList<CardInfoPtr> getCardsMatchingGroup(QList<CardInfoPtr> cardsToSort);
    void resizeEvent(QResizeEvent *event) override;

    DeckListModel *deckListModel;
    QString zoneName;
    QString cardGroupCategory;
    QString activeGroupCriteria;
    QStringList activeSortCriteria;

public slots:
    QList<CardInfoPtr> sortCardList(QList<CardInfoPtr> cardsToSort, QStringList properties, Qt::SortOrder order);
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);
    void updateCardDisplays();
    void onActiveSortCriteriaChanged(QStringList activeSortCriteria);

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(CardInfoPtr card);

private:
    QVBoxLayout *layout;
    BannerWidget *banner;
    OverlapWidget *overlapWidget;
};

#endif // OVERLAPPED_CARD_GROUP_DISPLAY_WIDGET_H
