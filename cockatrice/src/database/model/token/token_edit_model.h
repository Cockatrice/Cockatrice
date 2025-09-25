#ifndef COCKATRICE_TOKEN_EDIT_MODEL_H
#define COCKATRICE_TOKEN_EDIT_MODEL_H

#include "../database/card_database_display_model.h"

class TokenEditModel : public CardDatabaseDisplayModel
{
    Q_OBJECT
public:
    explicit TokenEditModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // COCKATRICE_TOKEN_EDIT_MODEL_H
