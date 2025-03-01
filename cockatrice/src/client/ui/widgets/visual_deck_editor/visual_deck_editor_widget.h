#ifndef VISUAL_DECK_EDITOR_H
#define VISUAL_DECK_EDITOR_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"

#include <QWidget>
#include <qscrollarea.h>

class VisualDeckEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckEditorWidget(QWidget *parent, DeckListModel *deckListModel);
    void resizeEvent(QResizeEvent *event) override;
    void updateDisplay();
    QWidget *displayCards(QList<QPair<QString, QList<CardInfoPtr>>> cardLists, int bannerOpacity);
    QList<QPair<QString, QList<CardInfoPtr>>> sortCards(QList<CardInfoPtr> *cardsToSort);

    OverlapWidget *getSortedCards();
    void populateCards();
    void sortCardList(QStringList properties, Qt::SortOrder order);
    void setDeckList(const DeckList &new_deck_list_model);

public slots:
    void decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight);

signals:
    void activeCardChanged(CardInfoPtr activeCard);
    void mainboardCardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void sideboardCardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

protected slots:
    void onHover(CardInfoPtr hoveredCard);
    void onMainboardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onSideboardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void activeSortCriteriaChanged();

private:
    DeckListModel *deckListModel;
    QList<CardInfoPtr> *mainDeckCards;
    QList<CardInfoPtr> *sideboardCards;
    QVBoxLayout *mainLayout;
    QComboBox *sortByComboBox;
    QString activeSortCriteria = "maintype";
    FlowWidget *flowWidget;
    QScrollArea *scrollArea;
    QWidget *zoneContainer;
    QVBoxLayout *zoneContainerLayout;
    OverlapControlWidget *overlap_control_widget;
    QWidget *container;
};

#endif // VISUAL_DECK_EDITOR_H
