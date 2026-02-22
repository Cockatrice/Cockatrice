/**
 * @file visual_deck_editor_widget.h
 * @ingroup DeckEditors
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_EDITOR_H
#define VISUAL_DECK_EDITOR_H

#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"
#include "../general/tutorial/tutorial_controller.h"
#include "../quick_settings/settings_button_widget.h"
#include "visual_deck_editor_placeholder_widget.h"

#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card_database_display_model.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <qscrollarea.h>

class VisualDeckDisplayOptionsWidget;
class DeckCardZoneDisplayWidget;
enum class DisplayType
{
    Flat,
    Overlap
};

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
    TutorialSequence addTutorialSteps();
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
