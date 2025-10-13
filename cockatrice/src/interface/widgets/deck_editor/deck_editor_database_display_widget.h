/**
 * @file deck_editor_database_display_widget.h
 * @ingroup DeckEditorWidgets
 * @ingroup CardDatabaseWidgets
 * @brief A Deck Editor Widget that displays a QTreeView of a CardDatabaseModel.
 */

#ifndef DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
#define DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H

#include "../../utility/key_signals.h"
#include "../tabs/abstract_tab_deck_editor.h"
#include "../utility/custom_line_edit.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/card/database/model/card_database_display_model.h>
#include <libcockatrice/card/database/model/card_database_model.h>

class AbstractTabDeckEditor;
class DeckEditorDatabaseDisplayWidget : public QWidget
{

    Q_OBJECT
public:
    explicit DeckEditorDatabaseDisplayWidget(AbstractTabDeckEditor *parent);
    AbstractTabDeckEditor *deckEditor;
    SearchLineEdit *searchEdit;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;

public slots:
    ExactCard currentCard() const;
    ExactCard getCardOrPinnedPrinting(QString cardName) const;
    void setFilterTree(FilterTree *filterTree);
    void clearAllDatabaseFilters();

signals:
    void addCardToMainDeck(const ExactCard &card);
    void addCardToSideboard(const ExactCard &card);
    void decrementCardFromMainDeck(const ExactCard &card);
    void decrementCardFromSideboard(const ExactCard &card);
    void cardChanged(const ExactCard &_card);

private:
    KeySignals searchKeySignals;
    QTreeView *databaseView;
    QHBoxLayout *searchLayout;
    QAction *aAddCard, *aAddCardToSideboard;
    QVBoxLayout *centralFrame;
    QWidget *centralWidget;

private slots:
    void retranslateUi();
    void updateSearch(const QString &search);
    void updateCard(const QModelIndex &current, const QModelIndex &);
    void actAddCardToMainDeck();
    void actAddCardToSideboard();
    void actDecrementCardFromMainDeck();
    void actDecrementCardFromSideboard();
    void databaseCustomMenu(QPoint point);
    void copyDatabaseCellContents();
    void saveDbHeaderState();
};

#endif // DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
