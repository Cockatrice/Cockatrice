/**
 * @file visual_deck_editor_widget.h
 * @ingroup DeckEditors
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_EDITOR_H
#define VISUAL_DECK_EDITOR_H

#include "visual_deck_editor_display_type.h"

#include <QHash>
#include <QWidget>

class DeckListModel;
class QItemSelectionModel;
class DeckList;
class QItemSelection;
class QLineEdit;
class CardSizeWidget;
class CardInfoPictureWithTextOverlayWidget;
class ExactCard;
class QPushButton;
class QCompleter;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class CardCompleterProxyModel;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class VisualDeckDisplayOptionsWidget;
class VisualDeckEditorPlaceholderWidget;
class DeckCardZoneDisplayWidget;
class VisualDeckEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckEditorWidget(QWidget *parent, DeckListModel *deckListModel, QItemSelectionModel *selectionModel);
    void retranslateUi();
    void clearAllDisplayWidgets();

    void setDeckList(const DeckList &_deckListModel);

    void setSelectionModel(QItemSelectionModel *model);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void updatePlaceholderVisibility();
    QItemSelectionModel *getSelectionModel() const
    {
        return selectionModel;
    }

    QLineEdit *searchBar;
    CardSizeWidget *cardSizeWidget;

public slots:
    void decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight);
    void updateZoneWidgets();
    void cleanupInvalidZones(DeckCardZoneDisplayWidget *displayWidget);
    void onCardAddition(const QModelIndex &parent, int first, int last);
    void onCardRemoval(const QModelIndex &parent, int first, int last);
    void constructZoneWidgetForIndex(QPersistentModelIndex persistent);
    void constructZoneWidgetsFromDeckListModel();

signals:
    void activeCardChanged(const ExactCard &activeCard);
    void activeGroupCriteriaChanged(QString activeGroupCriteria);
    void activeSortCriteriaChanged(QStringList activeSortCriteria);
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void cardAdditionRequested(const ExactCard &card);
    void displayTypeChanged(DisplayType displayType);

protected:
    void initializeSearchBarAndCompleter();
    void initializeDisplayOptionsWidget();
    void initializeDisplayOptionsAndSearchWidget();
    void initializeScrollAreaAndZoneContainer();
    void connectDeckListModel();

protected slots:
    void onHover(const ExactCard &hoveredCard);
    void onCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void decklistModelReset();

private:
    DeckListModel *deckListModel;
    QItemSelectionModel *selectionModel;
    QVBoxLayout *mainLayout;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;
    CardCompleterProxyModel *proxyModel;
    QCompleter *completer;
    QWidget *displayOptionsAndSearch;
    QHBoxLayout *displayOptionsAndSearchLayout;
    VisualDeckDisplayOptionsWidget *displayOptionsWidget;
    QPushButton *searchPushButton;
    QScrollArea *scrollArea;
    QWidget *zoneContainer;
    QVBoxLayout *zoneContainerLayout;
    VisualDeckEditorPlaceholderWidget *placeholderWidget;
    // OverlapControlWidget *overlapControlWidget;
    QHash<QPersistentModelIndex, QWidget *> indexToWidgetMap;
};

#endif // VISUAL_DECK_EDITOR_H
