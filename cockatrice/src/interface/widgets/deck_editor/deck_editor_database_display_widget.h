/**
 * @file deck_editor_database_display_widget.h
 * @ingroup DeckEditorWidgets
 * @ingroup CardDatabaseWidgets
 * @brief A Deck Editor Widget that displays a QTreeView of a CardDatabaseModel.
 */

#ifndef DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
#define DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H

#include "../../key_signals.h"

#include <QWidget>

class SearchLineEdit;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class QTreeView;
class ExactCard;
class FilterTree;
class QHBoxLayout;
class QVBoxLayout;
class AbstractTabDeckEditor;
class DeckEditorDatabaseDisplayWidget : public QWidget
{

    Q_OBJECT
public:
    explicit DeckEditorDatabaseDisplayWidget(QWidget *parent, AbstractTabDeckEditor *deckEditor);
    AbstractTabDeckEditor *deckEditor;
    SearchLineEdit *searchEdit;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;

    QTreeView *getDatabaseView()
    {
        return databaseView;
    }

public slots:
    ExactCard currentCard() const;
    void setFilterTree(FilterTree *filterTree);
    void clearAllDatabaseFilters();
    void updateSearch(const QString &search);
    void updateCard(const QModelIndex &current, const QModelIndex &);
    void actAddCardToMainDeck();
    void actAddCardToSideboard();
    void actDecrementCardFromMainDeck();
    void actDecrementCardFromSideboard();
    void databaseCustomMenu(QPoint point);
    void copyDatabaseCellContents();

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
    void saveDbHeaderState();
};

#endif // DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
