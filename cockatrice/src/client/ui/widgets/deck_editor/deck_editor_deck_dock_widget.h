#ifndef DECK_EDITOR_DECK_DOCK_WIDGET_H
#define DECK_EDITOR_DECK_DOCK_WIDGET_H

#include "../../../../deck/custom_line_edit.h"
#include "../../../../game/cards/card_info.h"
#include "../../../game_logic/key_signals.h"
#include "../../../tabs/abstract_tab_deck_editor.h"
#include "../visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTreeView>

class DeckListModel;
class AbstractTabDeckEditor;
class DeckEditorDeckDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorDeckDockWidget(AbstractTabDeckEditor *parent);
    DeckListModel *deckModel;
    QTreeView *deckView;
    QComboBox *bannerCardComboBox;
    void createDeckDock();
    CardInfoPtr getCurrentCard();
    void retranslateUi();
    QString getDeckName()
    {
        return nameEdit->text();
    }
    QString getSimpleDeckName()
    {
        return nameEdit->text().simplified();
    }

public slots:
    void cleanDeck();
    void updateBannerCardComboBox();
    void setDeck(DeckLoader *_deck);
    DeckLoader *getDeckList();
    void actIncrement();
    bool swapCard(const QModelIndex &idx);
    void actDecrementCard(CardInfoPtr info, QString zoneName);
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
    void cardChanged(CardInfoPtr _card);

private:
    AbstractTabDeckEditor *deckEditor;
    KeySignals deckViewKeySignals;
    QLabel *nameLabel;
    LineEditUnfocusable *nameEdit;
    SettingsButtonWidget *quickSettingsWidget;
    QCheckBox *showBannerCardCheckBox;
    QCheckBox *showTagsWidgetCheckBox;
    QLabel *commentsLabel;
    QTextEdit *commentsEdit;
    QLabel *bannerCardLabel;
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget;
    QLabel *hashLabel1;
    LineEditUnfocusable *hashLabel;
    QLabel *activeGroupCriteriaLabel;
    QComboBox *activeGroupCriteriaComboBox;

    QAction *aRemoveCard, *aIncrement, *aDecrement, *aSwapCard;

    void recursiveExpand(const QModelIndex &index);
    QModelIndexList getSelectedCardNodes() const;

private slots:
    void decklistCustomMenu(QPoint point);
    void updateCard(QModelIndex, const QModelIndex &current);
    void updateName(const QString &name);
    void updateComments();
    void setBannerCard(int);
    void updateHash();
    void refreshShortcuts();
    void updateShowBannerCardComboBox(bool visible);
    void updateShowTagsWidget(bool visible);
};

#endif // DECK_EDITOR_DECK_DOCK_WIDGET_H
