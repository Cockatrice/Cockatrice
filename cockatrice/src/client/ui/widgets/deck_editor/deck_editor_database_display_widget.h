#ifndef DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
#define DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H

#include "../../../../deck/custom_line_edit.h"
#include "../../../../game/cards/card_database_model.h"
#include "../../../game_logic/key_signals.h"
#include "../../../tabs/tab_generic_deck_editor.h"

#include <QHBoxLayout>
#include <QWidget>

class TabGenericDeckEditor;
class DeckEditorDatabaseDisplayWidget : public QWidget
{

    Q_OBJECT
public:
    explicit DeckEditorDatabaseDisplayWidget(QWidget *parent, TabGenericDeckEditor *_deckEditor);
    TabGenericDeckEditor *deckEditor;
    SearchLineEdit *searchEdit;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;

public slots:
    CardInfoPtr currentCardInfo() const;
    void setFilterTree(FilterTree *filterTree);

private:
    KeySignals searchKeySignals;
    QTreeView *databaseView;
    QHBoxLayout *searchLayout;
    QAction *aAddCard, *aAddCardToSideboard;
    QVBoxLayout *centralFrame;
    QWidget *centralWidget;

private slots:
    void showSearchSyntaxHelp();
    void retranslateUi();
    void updateSearch(const QString &search);
    void databaseCustomMenu(QPoint point);
    void copyDatabaseCellContents();
    void saveDbHeaderState();
};

#endif // DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
