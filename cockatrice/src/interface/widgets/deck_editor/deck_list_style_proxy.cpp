#include "deck_list_style_proxy.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <libcockatrice/models/deck_list/deck_list_model.h>

QVariant DeckListStyleProxy::data(const QModelIndex &index, int role) const
{
    QModelIndex src = mapToSource(index);
    if (!src.isValid())
        return {};

    QVariant value = QIdentityProxyModel::data(index, role);

    bool isCard = src.data(DeckRoles::IsCardRole).toBool();

    if (role == Qt::FontRole && !isCard) {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role == Qt::BackgroundRole) {
        if (isCard) {
            const bool legal = QIdentityProxyModel::data(index, DeckRoles::IsLegalRole).toBool();
            int base = 255 - (index.row() % 2) * 30;
            return legal ? QBrush(QColor(base, base, base)) : QBrush(QColor(255, base / 3, base / 3));
        } else {
            int depth = src.data(DeckRoles::DepthRole).toInt();
            int color = 90 + 60 * depth;
            return QBrush(QColor(color, 255, color));
        }
    }

    if (role == Qt::ForegroundRole) {
        return QBrush(QColor(0, 0, 0));
    }

    return value;
}
