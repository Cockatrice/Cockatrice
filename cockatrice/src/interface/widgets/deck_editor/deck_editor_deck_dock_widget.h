/**
 * @file deck_editor_deck_dock_widget.h
 * @ingroup DeckEditorWidgets
 * @brief A Deck Editor DockWidget that displays several Qt Editors to modify various deck attributes as well as a
 * QTreeView of the DeckListModel.
 */

#ifndef DECK_EDITOR_DECK_DOCK_WIDGET_H
#define DECK_EDITOR_DECK_DOCK_WIDGET_H

#include "../../key_signals.h"
#include "libcockatrice/card/printing/exact_card.h"

#include <QDockWidget>
#include <QModelIndexList>
#include <QString>
#include <QTreeView>
#include <qtmetamacros.h>

class CommanderBracketWidget;
class DeckListModel;
class AbstractTabDeckEditor;
class DeckListHistoryManagerWidget;
class DeckListStyleProxy;
class DeckPreviewDeckTagsDisplayWidget;
class DeckStateManager;
class LineEditUnfocusable;
class QAction;
class QCheckBox;
class QComboBox;
class QItemSelectionModel;
class QLabel;
class QMenu;
class QModelIndex;
class QPoint;
class QPushButton;
class QTextEdit;
class QTimer;
class SettingsButtonWidget;

class DeckEditorDeckDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorDeckDockWidget(AbstractTabDeckEditor *parent);

    DeckListStyleProxy *proxy;
    QTreeView *deckView;
    QComboBox *bannerCardComboBox;
    QLabel *playmatLabel;
    QPushButton *playmatSettingsButton;
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

    CommanderBracketWidget *commanderBracketWidget;

    DeckListModel *getModel() const;
    [[nodiscard]] QModelIndexList getSelectedCardNodeSourceIndices() const;
    void offsetCountAtIndex(const QModelIndex &idx, bool isIncrement);

    void addMoveToZoneMenu(QMenu *menu, const QModelIndex &sourceCardIndex, const QString &currentBoardName);
    void addChangeBoardMenu(QMenu *menu, const QString &zoneName);
    QString createNewCustomZone(const QString &initialBoardName = {});
    void addNewZoneAction(QMenu *menu, const QString &initialBoardName = {});

private slots:
    void decklistCustomMenu(QPoint point);
    void updateCard(QModelIndex, const QModelIndex &current);
    void writeName();
    void writeComments();
    void writeBannerCard(int);
    void openPlaymatSettings();
    void updatePlaymatLabel();
    void applyActiveGroupCriteria();
    void setSelectedIndex(const QModelIndex &newCardIndex, bool preserveWidgetFocus);
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
