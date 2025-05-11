#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "../../../../deck/custom_line_edit.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_model.h"
#include "../../../../game/filters/filter_tree_model.h"
#include "../../../game_logic/key_signals.h"
#include "../../../tabs/abstract_tab_deck_editor.h"
#include "../../layouts/flow_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_save_load_widget.h"
#include "visual_database_display_main_type_filter_widget.h"
#include "visual_database_display_name_filter_widget.h"
#include "visual_database_display_set_filter_widget.h"
#include "visual_database_display_sub_type_filter_widget.h"

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
                                         AbstractTabDeckEditor *deckEditor,
                                         CardDatabaseModel *database_model,
                                         CardDatabaseDisplayModel *database_display_model);
    void retranslateUi();

    void adjustCardsPerPage();
    void populateCards();
    void loadNextPage();
    void loadCurrentPage();
    void sortCardList(const QStringList &properties, Qt::SortOrder order) const;
    void setDeckList(const DeckList &new_deck_list_model);

    QWidget *searchContainer;
    QHBoxLayout *searchLayout;
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
    void onHover(const CardInfoPtr &hoveredCard);
    void addCard(const CardInfoPtr &cardToAdd);
    void databaseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void wheelEvent(QWheelEvent *event) override;
    void modelDirty() const;
    void updateSearch(const QString &search) const;

private:
    QToolButton *clearFilterWidget;
    QWidget *filterContainer;
    QHBoxLayout *filterContainerLayout;
    SettingsButtonWidget *quickFilterSaveLoadWidget;
    VisualDatabaseDisplayFilterSaveLoadWidget *saveLoadWidget;
    SettingsButtonWidget *quickFilterNameWidget;
    VisualDatabaseDisplayNameFilterWidget *nameFilterWidget;
    VisualDatabaseDisplayMainTypeFilterWidget *mainTypeFilterWidget;
    SettingsButtonWidget *quickFilterSubTypeWidget;
    VisualDatabaseDisplaySubTypeFilterWidget *subTypeFilterWidget;
    SettingsButtonWidget *quickFilterSetWidget;
    VisualDatabaseDisplaySetFilterWidget *setFilterWidget;
    KeySignals searchKeySignals;
    AbstractTabDeckEditor *deckEditor;
    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    QTreeView *databaseView;
    QList<CardInfoPtr> *cards;
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
