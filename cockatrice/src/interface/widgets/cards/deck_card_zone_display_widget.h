/**
 * @file deck_card_zone_display_widget.h
 * @ingroup DeckEditorWidgets
 */
//! \todo Document this file.

#ifndef DECK_CARD_ZONE_DISPLAY_WIDGET_H
#define DECK_CARD_ZONE_DISPLAY_WIDGET_H

#include "../visual_deck_editor/visual_deck_editor_widget.h"

#include <QHash>
#include <QList>
#include <QMouseEvent>
#include <QPersistentModelIndex>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class BannerWidget;
class CardGroupDisplayWidget;
class CardSizeWidget;
class DeckListModel;
class ExactCard;
class OverlapWidget;
class QItemSelection;
class QItemSelectionModel;
class QModelIndex;
class QVBoxLayout;

class DeckCardZoneDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DeckCardZoneDisplayWidget(QWidget *parent,
                              DeckListModel *deckListModel,
                              QItemSelectionModel *selectionModel,
                              QPersistentModelIndex trackedIndex,
                              QString zoneName,
                              QString activeGroupCriteria,
                              QStringList activeSortCriteria,
                              DisplayType displayType,
                              int bannerOpacity,
                              int subBannerOpacity,
                              CardSizeWidget *_cardSizeWidget);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    DeckListModel *deckListModel;
    QItemSelectionModel *selectionModel;
    QPersistentModelIndex trackedIndex;
    QString zoneName;
    void addCardsToOverlapWidget();

public slots:
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
    void updateZoneCardCount();

signals:
    void cardClicked(QMouseEvent *event, const ExactCard &card, const QString &zoneName);
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
