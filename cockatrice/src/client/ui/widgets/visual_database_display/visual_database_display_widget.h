#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_model.h"
#include "../../layouts/flow_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"

#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>
#include <qscrollarea.h>

class VisualDatabaseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayWidget(QWidget *parent,
                                         CardDatabaseModel *database_model,
                                         CardDatabaseDisplayModel *database_display_model);
    void updateDisplay();
    void adjustCardsPerPage();
    void populateCards();
    void searchModelChanged();
    void loadCurrentPage();
    void sortCardList(QStringList properties, Qt::SortOrder order);
    void setDeckList(const DeckList &new_deck_list_model);

signals:
    void cardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void cardHoveredDatabaseDisplay(CardInfoPtr hoveredCard);

protected slots:
    void onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onHover(CardInfoPtr hoveredCard);
    void databaseDataChanged(QModelIndex topLeft, QModelIndex bottomRight);
    void wheelEvent(QWheelEvent *event) override;
    void setupPaginationControls();
    void modelDirty();

private:
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    QList<CardInfoPtr> *cards;
    QVBoxLayout *main_layout;
    QScrollArea *scrollArea;
    FlowWidget *flow_widget;
    QWidget *overlap_categories;
    QVBoxLayout *overlap_categories_layout;
    OverlapControlWidget *overlap_control_widget;
    QWidget *container;
    QTimer *debounce_timer;

    bool isAnimating = false;
    int debounce_time = 300;
    int currentPage = 0;  // Current page index
    int cardsPerPage = 9; // Number of cards per page
    int cardsPerRow = 0;
    int rowsPerColumn = 0;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
