#ifndef CARD_GROUP_DISPLAY_WIDGET_H
#define CARD_GROUP_DISPLAY_WIDGET_H

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database.h"
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
                           QString zoneName,
                           QString cardGroupCategory,
                           QString activeGroupCriteria,
                           QStringList activeSortCriteria,
                           int bannerOpacity,
                           CardSizeWidget *cardSizeWidget);

    QList<CardInfoPtr> getCardsMatchingGroup(QList<CardInfoPtr> cardsToSort);
    void resizeEvent(QResizeEvent *event) override;

    DeckListModel *deckListModel;
    QString zoneName;
    QString cardGroupCategory;
    QString activeGroupCriteria;
    QStringList activeSortCriteria;
    CardSizeWidget *cardSizeWidget;

public slots:
    QList<CardInfoPtr> sortCardList(QList<CardInfoPtr> cardsToSort, QStringList properties, Qt::SortOrder order);
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void onHover(CardInfoPtr card);
    virtual void updateCardDisplays();
    void onActiveSortCriteriaChanged(QStringList activeSortCriteria);

signals:
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card);
    void cardHovered(CardInfoPtr card);

protected:
    QVBoxLayout *layout;
    BannerWidget *banner;
};
#endif // CARD_GROUP_DISPLAY_WIDGET_H
