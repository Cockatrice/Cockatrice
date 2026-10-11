/**
 * @file visual_database_display_widget.h
 * @ingroup VisualCardDatabaseWidgets
 */
//! \todo Document this file.

#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "libcockatrice/card/card_info.h"

#include <QLoggingCategory>
#include <QMouseEvent>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>
#include <qnamespace.h>
#include <qtmetamacros.h>

class CardDatabaseDisplayModel;
class CardDatabaseModel;
class CardDatabaseView;
class CardSizeWidget;
class DeckList;
class DeckListModel;
class ExactCard;
class FilterTreeModel;
class FlowWidget;
class OverlapControlWidget;
class QLabel;
class QModelIndex;
class QPushButton;
class QScrollArea;
class QTimer;
class QToolButton;
class QVBoxLayout;
class SearchLineEdit;
class VisualDatabaseDisplayColorFilterWidget;
class VisualDatabaseDisplayFilterToolbarWidget;
template <typename T> class QList;

inline Q_LOGGING_CATEGORY(VisualDatabaseDisplayLog, "visual_database_display");

class VisualDatabaseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayWidget(QWidget *parent,
                                         CardDatabaseModel *database_model,
                                         DeckListModel *deckListModel = nullptr);
    void retranslateUi();

    void adjustCardsPerPage();
    void populateCards();
    void loadPage(int start, int end);
    void loadNextPage();
    void loadCurrentPage();
    void sortCardList(const QStringList &properties, Qt::SortOrder order) const;
    void setDeckList(const DeckList &new_deck_list_model);

    /**
     * @brief Sets the callback used to create a custom zone from the add-to-zone menu.
     * The callback returns the name of the created zone, or an empty string if creation was cancelled.
     */
    void setNewZoneCreator(const std::function<QString()> &creator);

    CardDatabaseDisplayModel *getDatabaseDisplayModel()
    {
        return databaseDisplayModel;
    }

    CardDatabaseView *getDatabaseView()
    {
        return databaseView;
    }

    FilterTreeModel *getFilterModel()
    {
        return filterModel;
    }

    /**
     * @return False if the widget is in database display mode and true if it's in visual display mode
     */
    bool isVisualDisplayMode() const;

public slots:
    void onSearchModelChanged();

signals:
    void cardClickedDatabaseDisplay(QMouseEvent *event, const ExactCard &card);
    void cardHoveredDatabaseDisplay(const ExactCard &hoveredCard);

    void cardAdded(const ExactCard &card, const QString &zoneName);
    void cardDecremented(const ExactCard &card, const QString &zoneName);
    void edhrecRequested(const CardInfoPtr &cardInfo, bool isCommander);
    void printingSelectorRequested();
    void cardInfoRequested(const ExactCard &cardName);

protected slots:
    void initialize();
    void onClick(QMouseEvent *event, const ExactCard &card);
    void onHover(const ExactCard &hoveredCard);
    void addCardToDisplay(const ExactCard &cardToAdd);
    void databaseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void modelDirty() const;
    void onDisplayModeChanged(bool checked);

    void onSelectedCardChanged(const QString &cardName);
    void actAddCard(const QString &cardName, const QString &zoneName);
    void actDecrementCard(const QString &cardName, const QString &zoneName);
    void onRelatedCardClicked(const QString &relatedCard);

private:
    FlowWidget *searchContainer;
    SearchLineEdit *searchEdit;
    QPushButton *displayModeButton;
    FilterTreeModel *filterModel;
    VisualDatabaseDisplayColorFilterWidget *colorFilterWidget;

    QLabel *databaseLoadIndicator;

    QToolButton *clearFilterWidget;
    VisualDatabaseDisplayFilterToolbarWidget *filterContainer;
    CardDatabaseDisplayModel *databaseDisplayModel;
    CardDatabaseView *databaseView;
    std::function<QString()> newZoneCreator;
    QList<ExactCard> *cards;
    QVBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    FlowWidget *flowWidget;
    QWidget *overlapCategories;
    QVBoxLayout *overlapCategoriesLayout;
    OverlapControlWidget *overlapControlWidget;
    CardSizeWidget *cardSizeWidget;
    QTimer *debounceTimer;

    int debounceTime = 300; // in Ms
    int currentPage = 0;    // Current page index
    int cardsPerPage = 100; // Number of cards per page
    bool filtersInitialized = false;
    bool initialLoadScheduled = false;

    void initializeFilters();
    void highlightAllSearchEdit();
    bool nearEndOfPage() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
