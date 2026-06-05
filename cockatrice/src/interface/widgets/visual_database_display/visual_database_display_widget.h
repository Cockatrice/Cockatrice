/**
 * @file visual_database_display_widget.h
 * @ingroup VisualCardDatabaseWidgets
 */
//! \todo Document this file.

#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "../../../filters/filter_tree_model.h"
#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../../key_signals.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"
#include "../utility/custom_line_edit.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_toolbar_widget.h"

#include <QLoggingCategory>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <qscrollarea.h>

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

    void highlightAllSearchEdit();
    bool nearEndOfPage() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
