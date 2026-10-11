/**
 * @file deck_editor_database_display_widget.h
 * @ingroup DeckEditorWidgets
 * @ingroup CardDatabaseWidgets
 * @brief A Deck Editor Widget that displays a QTreeView of a CardDatabaseModel.
 */

#ifndef DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
#define DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H

#include "libcockatrice/card/card_info.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class CardDatabaseView;
class CardDatabaseDisplayModel;
class CardDatabaseModel;
class ExactCard;
class FilterTree;
class QAction;
class QHBoxLayout;
class QVBoxLayout;
class SearchLineEdit;

class DeckEditorDatabaseDisplayWidget : public QWidget
{

    Q_OBJECT
public:
    explicit DeckEditorDatabaseDisplayWidget(QWidget *parent, CardDatabaseModel *databaseModel);

    CardDatabaseView *getDatabaseView() const
    {
        return databaseView;
    }

public slots:
    void setFilterTree(FilterTree *filterTree);
    void clearAllDatabaseFilters();

    void actAddCardToMainDeck();
    void actAddCardToSideboard();

    void addCard(const QString &cardName, const QString &zoneName);
    void decrementCard(const QString &cardName, const QString &zoneName);
    void updateCard(const QString &cardName);

signals:
    void cardAdded(const ExactCard &card, const QString &zoneName);
    void cardDecremented(const ExactCard &card, const QString &zoneName);
    void cardChanged(const ExactCard &_card);

    void edhrecRequested(const CardInfoPtr &cardInfo, bool isCommander);
    void printingSelectorRequested();
    void cardInfoRequested(const ExactCard &card);

private:
    CardDatabaseDisplayModel *databaseDisplayModel;
    CardDatabaseView *databaseView;
    QHBoxLayout *searchLayout;
    SearchLineEdit *searchEdit;
    QAction *aAddCard, *aAddCardToSideboard;
    QVBoxLayout *centralFrame;
    QWidget *centralWidget;

    void highlightAllSearchEdit();

private slots:
    void retranslateUi();

    void onRelatedCardClicked(const QString &relatedCard);
};

#endif // DECK_EDITOR_DATABASE_DISPLAY_WIDGET_H
