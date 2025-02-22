#ifndef TAB_GENERIC_DECK_EDITOR_H
#define TAB_GENERIC_DECK_EDITOR_H

#include "../../deck/custom_line_edit.h"
#include "../../game/cards/card_database.h"
#include "../menus/deck_editor/deck_editor_menu.h"
#include "../ui/widgets/deck_editor/deck_editor_card_info_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_deck_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_filter_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_printing_selector_dock_widget.h"
#include "../ui/widgets/printing_selector/printing_selector.h"
#include "../ui/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "tab.h"

#include <QAbstractItemModel>
#include <QDir>

class CardDatabaseModel;
class CardDatabaseDisplayModel;

class QTreeView;

class DeckEditorMenu;

class CardInfoFrameWidget;
class QTextEdit;
class QLabel;
class DeckLoader;
class DeckEditorDeckDockWidget;
class DeckEditorFilterDockWidget;
class DeckEditorPrintingSelectorDockWidget;
class DeckPreviewDeckTagsDisplayWidget;
class Response;
class FilterTreeModel;
class FilterBuilder;
class QComboBox;
class QGroupBox;
class QMessageBox;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QDockWidget;

class TabGenericDeckEditor : public Tab
{
    Q_OBJECT
protected slots:
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);             // generic
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);            // generic
    void updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex &previous); // possibly generic
    void updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex &previous); // possibly generic
    void updateSearch(const QString &search);                                                     // generic
    void databaseCustomMenu(QPoint point);                                                        // generic
    void decklistCustomMenu(QPoint point);

    virtual void actNewDeck() = 0;
    virtual void actLoadDeck() = 0;                     // NOT generic
    bool actSaveDeck();                                 // generic
    bool actSaveDeckAs();                               // generic
    void actLoadDeckFromClipboard();                    // generic
    void actSaveDeckToClipboard();                      // generic
    void actSaveDeckToClipboardNoSetNameAndNumber();    // generic
    void actSaveDeckToClipboardRaw();                   // generic
    void actSaveDeckToClipboardRawNoSetNameAndNumber(); // generic
    void actPrintDeck();                                // generic
    void actExportDeckDecklist();                       // generic
    void actAnalyzeDeckDeckstats();                     // generic
    void actAnalyzeDeckTappedout();                     // generic

    void actAddCard();                    // generic
    void actAddCardToSideboard();         // generic
    void actDecrementCard();              // generic
    void actDecrementCardFromSideboard(); // generic
    void copyDatabaseCellContents();      // generic

    void saveDeckRemoteFinished(const Response &r); // generic

    virtual void loadLayout() = 0;
    virtual void restartLayout() = 0; // somewhat generic
    virtual void freeDocksSize() = 0;
    virtual void refreshShortcuts() = 0;

    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void dockVisibleTriggered() = 0;             // custom
    virtual void dockFloatingTriggered() = 0;            // custom
    virtual void dockTopLevelChanged(bool topLevel) = 0; // custom
    void saveDbHeaderState();                            // generic
    void showSearchSyntaxHelp();                         // generic

protected:
    /**
     * @brief Which tab to open the new deck in
     */
    enum DeckOpenLocation
    {
        CANCELLED,
        SAME_TAB,
        NEW_TAB
    };
    DeckOpenLocation confirmOpen(const bool openInSameTabIfBlank = true);
    QMessageBox *createSaveConfirmationWindow();

    bool isBlankNewDeck() const;
    CardInfoPtr currentCardInfo() const;        // generic
    void decrementCardHelper(QString zoneName); // generic
    virtual void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation) = 0;

    CardDatabaseModel *databaseModel;
    QTreeView *databaseView;
    KeySignals searchKeySignals;

    QMenu *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu;
    QAction *aAddCard, *aAddCardToSideboard;
    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating, *aFilterDockVisible,
        *aFilterDockFloating, *aPrintingSelectorDockVisible, *aPrintingSelectorDockFloating;

    bool modified;
    QVBoxLayout *centralFrame;
    QHBoxLayout *searchLayout;
    QWidget *centralWidget;

public:
    explicit TabGenericDeckEditor(TabSupervisor *_tabSupervisor);
    virtual void retranslateUi() override = 0;
    [[nodiscard]] virtual QString getTabText() const override = 0; // Subclasses MUST implement this
    void setDeck(DeckLoader *_deckLoader);                         // generic enough
    void setModified(bool _windowModified);                        // generic
    bool confirmClose();                                           // generic
    virtual void createMenus() = 0;
    virtual void createCentralFrame() = 0;
    void updateCardInfo(CardInfoPtr _card);
    void addCardHelper(CardInfoPtr info, QString zoneName); // reasonably generic
    DeckEditorMenu *deckMenu;
    DeckEditorCardInfoDockWidget *cardInfoDockWidget;
    DeckEditorDeckDockWidget *deckDockWidget;
    DeckEditorFilterDockWidget *filterDockWidget;
    DeckEditorPrintingSelectorDockWidget *printingSelectorDockWidget;
    CardDatabaseDisplayModel *databaseDisplayModel;
    SearchLineEdit *searchEdit;

public slots:
    void actOpenRecent(const QString &fileName);     // generic
    void closeRequest(bool forced = false) override; // generic
    virtual void showPrintingSelector() = 0;
signals:
    void openDeckEditor(const DeckLoader *deckLoader);
    void deckEditorClosing(TabGenericDeckEditor *tab);
};

#endif // TAB_GENERIC_DECK_EDITOR_H
