/**
 * @file card_completer_proxy_model.h
 * @ingroup CardDatabaseModels
 * @brief TODO: Document this.
 */

#ifndef CARD_COMPLETER_PROXY_MODEL_H
#define CARD_COMPLETER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class CardCompleterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CardCompleterProxyModel(QObject *parent = nullptr);

protected:
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // CARD_COMPLETER_PROXY_MODEL_H
