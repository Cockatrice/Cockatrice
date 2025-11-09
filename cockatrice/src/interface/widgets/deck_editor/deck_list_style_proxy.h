#ifndef COCKATRICE_DECK_LIST_STYLE_PROXY_H
#define COCKATRICE_DECK_LIST_STYLE_PROXY_H

#include <QIdentityProxyModel>

class DeckListStyleProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    using QIdentityProxyModel::QIdentityProxyModel;

    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // COCKATRICE_DECK_LIST_STYLE_PROXY_H
