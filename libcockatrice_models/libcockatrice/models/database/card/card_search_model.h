/**
 * @file card_search_model.h
 * @ingroup CardDatabaseModels
 */
//! \todo Document this file.

#ifndef CARD_SEARCH_MODEL_H
#define CARD_SEARCH_MODEL_H

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/card_localization.h"

#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <optional>
#include <qnamespace.h>
#include <qtmetamacros.h>

class CardDatabaseDisplayModel;
class QObject;

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
        int nameLength;
    };

    /**
     * @brief Finds the lowercased card name that matches the query.
     *
     * The completer suggestions match against the same languages the card
     * search uses, so typing a localized name finds the card. In Both mode
     * either language can match.
     *
     * @return The matched lowercased name, or std::nullopt when the card does not match.
     */
    [[nodiscard]] std::optional<QString> matchingLowercaseName(const CardInfoPtr &card,
                                                               const QString &lowerQuery) const;

    CardDatabaseDisplayModel *sourceModel;
    QList<SearchResult> searchResults;
    CardSearchLanguage searchLanguage;
};

#endif // CARD_SEARCH_MODEL_H
