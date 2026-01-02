/**
 * @file deck_editor_deck_dock_widget.h
 * @ingroup DeckEditorWidgets
 * @brief A Deck Editor DockWidget that displays several Qt Editors to modify various deck attributes as well as a
 * QTreeView of the DeckListModel.
 */

#ifndef DECK_EDITOR_DECK_DOCK_WIDGET_H
#define DECK_EDITOR_DECK_DOCK_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../../key_signals.h"
#include "../utility/custom_line_edit.h"
#include "../visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "deck_list_history_manager_widget.h"
#include "deck_list_style_proxy.h"

#include <QDockWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTreeView>
#include <libcockatrice/card/card_info.h>

class DeckListModel;
class AbstractTabDeckEditor;
class DeckEditorDeckDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorDeckDockWidget(AbstractTabDeckEditor *parent);

    DeckListStyleProxy *proxy;
    QTreeView *deckView;
    QComboBox *bannerCardComboBox;
    void createDeckDock();
    ExactCard getCurrentCard();
    void retranslateUi();

    QComboBox *getGroupByComboBox()
    {
        return activeGroupCriteriaComboBox;
    }

    [[nodiscard]] QItemSelectionModel *getSelectionModel() const
    {
        return deckView->selectionModel();
    }

public slots:
    void selectPrevCard();
    void selectNextCard();
    void updateBannerCardComboBox();
    void syncDisplayWidgetsToModel();
    void actAddCard(const ExactCard &card, const QString &zoneName);
    void actIncrementSelection();
    void actDecrementCard(const ExactCard &card, QString zoneName);
    void actDecrementSelection();
    void actSwapCard(const ExactCard &card, const QString &zoneName);
    void actSwapSelection();
    void actRemoveCard();
    void initializeFormats();

signals:
    void selectedCardChanged(const ExactCard &card);

private:
    AbstractTabDeckEditor *deckEditor;
    DeckStateManager *deckStateManager;

    DeckListHistoryManagerWidget *historyManagerWidget;
    KeySignals deckViewKeySignals;
    QLabel *nameLabel;
    LineEditUnfocusable *nameEdit;
    QTimer *nameDebounceTimer;
    SettingsButtonWidget *quickSettingsWidget;
    QCheckBox *showBannerCardCheckBox;
    QCheckBox *showTagsWidgetCheckBox;
    QLabel *commentsLabel;
    QTextEdit *commentsEdit;
    QTimer *commentsDebounceTimer;
    QLabel *bannerCardLabel;
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget;
    QLabel *hashLabel1;
    LineEditUnfocusable *hashLabel;
    QLabel *activeGroupCriteriaLabel;
    QComboBox *activeGroupCriteriaComboBox;
    QLabel *formatLabel;
    QComboBox *formatComboBox;

    QAction *aRemoveCard, *aIncrement, *aDecrement, *aSwapCard;

    DeckListModel *getModel() const;
    [[nodiscard]] QModelIndexList getSelectedCardNodeSourceIndices() const;
    void offsetCountAtIndex(const QModelIndex &idx, bool isIncrement);

private slots:
    void decklistCustomMenu(QPoint point);
    void updateCard(QModelIndex, const QModelIndex &current);
    void writeName();
    void writeComments();
    void writeBannerCard(int);
    void applyActiveGroupCriteria();
    void setSelectedIndex(const QModelIndex &newCardIndex);
    void updateHash();
    void refreshShortcuts();
    void updateShowBannerCardComboBox(bool visible);
    void updateShowTagsWidget(bool visible);
    void syncBannerCardComboBoxSelectionWithDeck();
    void changeSelectedCard(int changeBy);
    void recursiveExpand(const QModelIndex &parent);
    void expandAll();
};

#endif // DECK_EDITOR_DECK_DOCK_WIDGET_H
