/**
 * @file visual_database_display_widget.h
 * @ingroup VisualCardDatabaseWidgets
 */
//! \todo Document this file.

#ifndef VISUAL_DATABASE_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_WIDGET_H

#include "libcockatrice/card/card_info.h"

#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>
#include <qnamespace.h>
#include <qtmetamacros.h>

class CardDatabaseDisplayModel;
class CardDatabaseModel;
class CardDatabaseView;
class CardDatabaseVisualDisplayModel;
class CardDatabaseVisualView;
class CardSizeWidget;
class DeckListModel;
class ExactCard;
class FilterTreeModel;
class FlowWidget;
class QLabel;
class QMouseEvent;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class SearchLineEdit;
class VisualDatabaseDisplayColorFilterWidget;
class VisualDatabaseDisplayFilterToolbarWidget;

class VisualDatabaseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayWidget(QWidget *parent,
                                         CardDatabaseModel *database_model,
                                         DeckListModel *deckListModel = nullptr);
    void retranslateUi();

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
    /**
     * @brief Pushes the active set filters to the visual grid.
     *
     * The grid's rows follow the display model's filter and sort signals on their
     * own; this only forwards the set filters, which decide which printings the
     * grid shows.
     */
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
    CardDatabaseVisualDisplayModel *visualDisplayModel;
    CardDatabaseView *databaseView;
    CardDatabaseVisualView *cardView;
    std::function<QString()> newZoneCreator;
    QVBoxLayout *mainLayout;
    CardSizeWidget *cardSizeWidget;

    bool filtersInitialized = false;

    void initializeFilters();
    void highlightAllSearchEdit();
    void updateDisplayModeButton();
    [[nodiscard]] QStringList collectSetFilterTerms() const;

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DATABASE_DISPLAY_WIDGET_H
