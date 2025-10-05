/**
 * @file token_display_model.h
 * @ingroup CardDatabaseModels
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_TOKEN_DISPLAY_MODEL_H
#define COCKATRICE_TOKEN_DISPLAY_MODEL_H

#include "../card_database_display_model.h"

class TokenDisplayModel : public CardDatabaseDisplayModel
{
    Q_OBJECT
public:
    explicit TokenDisplayModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // COCKATRICE_TOKEN_DISPLAY_MODEL_H
