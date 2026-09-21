/**
 * @file card_search_model.h
 * @ingroup CardDatabaseModels
 */
//! \todo Document this file.

#ifndef CARD_SEARCH_MODEL_H
#define CARD_SEARCH_MODEL_H

#include "../card_database_display_model.h"

#include <QAbstractListModel>

class CardSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CardSearchRoles
    {
        CardInfoRole = Qt::UserRole + 1,
    };

    explicit CardSearchModel(CardDatabaseDisplayModel *sourceModel, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateSearchResults(const QString &query); // Update results based on input

    void setSearchLanguage(const CardSearchLanguage &searchLang)
    {
        if (searchLanguage == searchLang) {
            return;
        }
        searchLanguage = searchLang;
    }

private:
    struct SearchResult
    {
        CardInfoPtr card;
        int distance;
    };

    /** @brief The names a card is searched by with the current search language. */
    [[nodiscard]] QStringList searchableNames(const CardInfoPtr &card) const;

    /** @brief The name used to break distance ties when sorting suggestions. */
    [[nodiscard]] QString sortableName(const CardInfoPtr &card) const;

    CardDatabaseDisplayModel *sourceModel;
    QList<SearchResult> searchResults;
    CardSearchLanguage searchLanguage;
};

#endif // CARD_SEARCH_MODEL_H
