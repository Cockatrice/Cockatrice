#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "../../../../deck/custom_line_edit.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_model.h"
#include "../../../../game/filters/filter_tree_model.h"
#include "../../../game_logic/key_signals.h"
#include "../../layouts/flow_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"
#include "visual_database_display_color_filter_widget.h"

#include <QLoggingCategory>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>
#include <qscrollarea.h>

inline Q_LOGGING_CATEGORY(VisualDatabaseDisplayLog, "visual_database_display");

class VisualDatabaseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayWidget(QWidget *parent,
                                         CardDatabaseModel *database_model,
                                         CardDatabaseDisplayModel *database_display_model);

    void adjustCardsPerPage();
    void populateCards();
    void loadNextPage();
    void loadCurrentPage();
    void sortCardList(QStringList properties, Qt::SortOrder order);
    void setDeckList(const DeckList &new_deck_list_model);

    SearchLineEdit *searchEdit;
    FilterTreeModel *filterModel;
    VisualDatabaseDisplayColorFilterWidget *colorFilterWidget;

public slots:
    void searchModelChanged();

signals:
    void cardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void cardHoveredDatabaseDisplay(CardInfoPtr hoveredCard);

protected slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onHover(CardInfoPtr hoveredCard);
    void addCard(CardInfoPtr cardToAdd);
    void databaseDataChanged(QModelIndex topLeft, QModelIndex bottomRight);
    void wheelEvent(QWheelEvent *event) override;
    void modelDirty();
    void updateSearch(const QString &search);

private:
    KeySignals searchKeySignals;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    QTreeView *databaseView;
    QList<CardInfoPtr> *cards;
    QVBoxLayout *main_layout;
    QScrollArea *scrollArea;
    FlowWidget *flow_widget;
    QWidget *overlap_categories;
    QVBoxLayout *overlap_categories_layout;
    OverlapControlWidget *overlap_control_widget;
    CardSizeWidget *cardSizeWidget;
    QWidget *container;
    QTimer *debounce_timer;

    int debounce_time = 300;
    int currentPage = 0;    // Current page index
    int cardsPerPage = 100; // Number of cards per page

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
