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
    void setDeck(const LoadedDeck &_deck);
    void syncDisplayWidgetsToModel();
    void sortDeckModelToDeckView();
    DeckLoader *getDeckLoader();
    const DeckList &getDeckList() const;
    void actAddCard(const ExactCard &card, const QString &zoneName);
    void actIncrementSelection();
    void actDecrementCard(const ExactCard &card, QString zoneName);
    void actDecrementSelection();
    void actSwapCard(const ExactCard &card, const QString &zoneName);
    void actSwapSelection();
    void actRemoveCard();
    void initializeFormats();

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
    QLabel *formatLabel;
    QComboBox *formatComboBox;

    QAction *aRemoveCard, *aIncrement, *aDecrement, *aSwapCard;

    [[nodiscard]] QModelIndexList getSelectedCardNodes() const;
    void offsetCountAtIndex(const QModelIndex &idx, bool isIncrement);

private slots:
    void decklistCustomMenu(QPoint point);
    bool swapCard(const QModelIndex &currentIndex);
    void updateCard(QModelIndex, const QModelIndex &current);
    void updateName(const QString &name);
    void updateComments();
    void setBannerCard(int);
    void setTags(const QStringList &tags);
    void syncDeckListBannerCardWithComboBox();
    void updateHash();
    void refreshShortcuts();
    void updateShowBannerCardComboBox(bool visible);
    void updateShowTagsWidget(bool visible);
    void syncBannerCardComboBoxSelectionWithDeck();
    void recursiveExpand(const QModelIndex &parent);
    void expandAll();
};

#endif // DECK_EDITOR_DECK_DOCK_WIDGET_H
