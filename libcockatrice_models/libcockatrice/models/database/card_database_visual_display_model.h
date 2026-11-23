/**
 * @file card_database_visual_display_model.h
 * @ingroup CardDatabaseModels
 * @brief A list model of the exact cards a visual card grid paints.
 *
 * The filtered and sorted CardDatabaseDisplayModel holds one row per card name.
 * A visual grid shows one picture per *printing* instead, so this model flattens
 * the proxy rows into ExactCards: the preferred printing per card normally, and
 * every printing of the selected sets while set filters are active.
 */

#ifndef COCKATRICE_CARD_DATABASE_VISUAL_DISPLAY_MODEL_H
#define COCKATRICE_CARD_DATABASE_VISUAL_DISPLAY_MODEL_H

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/card/printing/printing_info.h"

#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QStringList>
#include <QTimer>
#include <qtmetamacros.h>

class CardDatabaseDisplayModel;
class CardDatabaseQuerier;

class CardDatabaseVisualDisplayModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Flattens _displayModel into the cards a visual grid displays.
     * @param _displayModel The filtered and sorted proxy model of the card database.
     * @param _querier Resolves the preferred printing of a card.
     * @param parent Parent object.
     */
    explicit CardDatabaseVisualDisplayModel(CardDatabaseDisplayModel *_displayModel,
                                            CardDatabaseQuerier *_querier,
                                            QObject *parent = nullptr);

    /**
     * @brief Sets the set shorthand names of the active set filters.
     *
     * With no terms every card is shown with its preferred printing. With terms
     * every printing belonging to one of the named sets is shown instead, in term
     * order. Rebuilds immediately when the terms change.
     */
    void setSetFilterTerms(const QStringList &terms);

    /**
     * @brief Rebuilds the row list from the proxy model right away.
     *
     * Proxy model changes normally rebuild on a short coalescing timer; call this
     * when the caller needs the new rows before returning.
     */
    void refresh();

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns the card painted at @p row, with its preferred printing resolved.
     * @param row Row in this model, not in the proxy model.
     */
    [[nodiscard]] ExactCard cardAt(int row) const;

private:
    struct Entry
    {
        CardInfoPtr info;
        /// Empty when the entry follows the preferred printing rather than a set filter.
        PrintingInfo printing;
    };

    void scheduleRebuild();
    void rebuild();
    void appendPrintingsFor(const CardInfoPtr &info);

    QPointer<CardDatabaseDisplayModel> displayModel;
    CardDatabaseQuerier *querier;
    QStringList setFilterTerms;
    QList<Entry> entries;
    QTimer rebuildTimer;
};

#endif // COCKATRICE_CARD_DATABASE_VISUAL_DISPLAY_MODEL_H
