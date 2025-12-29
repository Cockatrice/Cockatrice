/**
 * @file card_search_model.h
 * @ingroup CardDatabaseModels
 * @brief TODO: Document this.
 */

#ifndef CARD_SEARCH_MODEL_H
#define CARD_SEARCH_MODEL_H

#include "../card_database_display_model.h"

#include <QAbstractListModel>

class CardSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CardSearchModel(CardDatabaseDisplayModel *sourceModel, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateSearchResults(const QString &query); // Update results based on input

private:
    struct SearchResult
    {
        CardInfoPtr card;
        int distance;
    };

    CardDatabaseDisplayModel *sourceModel;
    QList<SearchResult> searchResults;
};

#endif // CARD_SEARCH_MODEL_H
