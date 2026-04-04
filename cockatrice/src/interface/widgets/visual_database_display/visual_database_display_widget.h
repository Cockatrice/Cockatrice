/**
 * @file visual_database_display_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "../../key_signals.h"

#include <QLoggingCategory>
#include <QWidget>

inline Q_LOGGING_CATEGORY(VisualDatabaseDisplayLog, "visual_database_display");

class AbstractTabDeckEditor;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckList;
class QTreeView;
class QHBoxLayout;
class SearchLineEdit;
class QPushButton;
class FilterTreeModel;
class VisualDatabaseDisplayColorFilterWidget;
class CardInfoPictureWithTextOverlayWidget;
class ExactCard;
class QLabel;
class QToolButton;
class VisualDatabaseDisplayFilterToolbarWidget;
class QVBoxLayout;
class QScrollArea;
class FlowWidget;
class OverlapControlWidget;
class CardSizeWidget;
class VisualDatabaseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayWidget(QWidget *parent,
                                         AbstractTabDeckEditor *deckEditor,
                                         CardDatabaseModel *database_model,
                                         CardDatabaseDisplayModel *database_display_model);
    void retranslateUi();

    void adjustCardsPerPage();
    void populateCards();
    void loadPage(int start, int end);
    void loadNextPage();
    void loadCurrentPage();
    void sortCardList(const QStringList &properties, Qt::SortOrder order) const;
    void setDeckList(const DeckList &new_deck_list_model);

    AbstractTabDeckEditor *getDeckEditor()
    {
        return deckEditor;
    }

    CardDatabaseDisplayModel *getDatabaseDisplayModel()
    {
        return databaseDisplayModel;
    }

    QTreeView *getDatabaseView()
    {
        return databaseView;
    }

    QWidget *searchContainer;
    QHBoxLayout *searchLayout;
    SearchLineEdit *searchEdit;
    QPushButton *displayModeButton;
    FilterTreeModel *filterModel;
    VisualDatabaseDisplayColorFilterWidget *colorFilterWidget;

public slots:
    void onSearchModelChanged();

signals:
    void cardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void cardHoveredDatabaseDisplay(const ExactCard &hoveredCard);

protected slots:
    void initialize();
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onHover(const ExactCard &hoveredCard);
    void addCard(const ExactCard &cardToAdd);
    void databaseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void wheelEvent(QWheelEvent *event) override;
    void modelDirty() const;
    void updateSearch(const QString &search) const;
    void onDisplayModeChanged(bool checked);

private:
    QLabel *databaseLoadIndicator;

    QToolButton *clearFilterWidget;
    VisualDatabaseDisplayFilterToolbarWidget *filterContainer;
    KeySignals searchKeySignals;
    AbstractTabDeckEditor *deckEditor;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    QTreeView *databaseView;
    QList<ExactCard> *cards;
    QVBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    FlowWidget *flowWidget;
    QWidget *overlapCategories;
    QVBoxLayout *overlapCategoriesLayout;
    OverlapControlWidget *overlapControlWidget;
    CardSizeWidget *cardSizeWidget;
    QTimer *debounceTimer;
    QTimer *loadCardsTimer;

    int debounceTime = 300; // in Ms
    int currentPage = 0;    // Current page index
    int cardsPerPage = 100; // Number of cards per page

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
