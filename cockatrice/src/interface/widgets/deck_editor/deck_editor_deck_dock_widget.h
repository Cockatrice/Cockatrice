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
    DeckLoader *deckLoader;
    DeckListStyleProxy *proxy;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QComboBox *bannerCardComboBox;
    void createDeckDock();
    ExactCard getCurrentCard();
    void retranslateUi();
    QString getDeckName()
    {
        return nameEdit->text();
    }
    QString getSimpleDeckName()
    {
        return nameEdit->text().simplified();
    }
    QComboBox *getGroupByComboBox()
    {
        return activeGroupCriteriaComboBox;
    }

    [[nodiscard]] QItemSelectionModel *getSelectionModel() const
    {
        return deckView->selectionModel();
    }

public slots:
    void cleanDeck();
    void updateBannerCardComboBox();
    void setDeck(DeckLoader *_deck);
    void syncDisplayWidgetsToModel();
    void sortDeckModelToDeckView();
    DeckLoader *getDeckLoader();
    DeckList *getDeckList();
    void actIncrement();
    bool swapCard(const QModelIndex &idx);
    void actDecrementCard(const ExactCard &card, QString zoneName);
    void actDecrementSelection();
    void actSwapCard();
    void actRemoveCard();
    void offsetCountAtIndex(const QModelIndex &idx, int offset);

signals:
    void nameChanged();
    void commentsChanged();
    void hashChanged();
    void deckChanged();
    void deckModified();
    void requestDeckHistorySave(const QString &modificationReason);
    void requestDeckHistoryClear();
    void cardChanged(const ExactCard &_card);

private:
    AbstractTabDeckEditor *deckEditor;
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

    QAction *aRemoveCard, *aIncrement, *aDecrement, *aSwapCard;

    void recursiveExpand(const QModelIndex &index);
    [[nodiscard]] QModelIndexList getSelectedCardNodes() const;

private slots:
    void decklistCustomMenu(QPoint point);
    void updateCard(QModelIndex, const QModelIndex &current);
    void updateName(const QString &name);
    void updateComments();
    void setBannerCard(int);
    void syncDeckListBannerCardWithComboBox();
    void updateHash();
    void refreshShortcuts();
    void updateShowBannerCardComboBox(bool visible);
    void updateShowTagsWidget(bool visible);
    void syncBannerCardComboBoxSelectionWithDeck();
    void expandAll();
};

#endif // DECK_EDITOR_DECK_DOCK_WIDGET_H
