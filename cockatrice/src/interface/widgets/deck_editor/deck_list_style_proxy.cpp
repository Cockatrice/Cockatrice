#include "deck_list_style_proxy.h"

#include "../../theme_manager.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QModelIndex>
#include <QVariant>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <qnamespace.h>

DeckListStyleProxy::DeckListStyleProxy(QObject *parent) : QIdentityProxyModel(parent)
{
    // FontRole data reads the DeckList role live; rebroadcast so attached
    // views re-query instead of waiting for an incidental repaint.
    connect(themeManager, &ThemeManager::fontsChanged, this, [this] {
        emit layoutAboutToBeChanged();
        emit layoutChanged();
    });
}

QVariant DeckListStyleProxy::data(const QModelIndex &index, int role) const
{
    QModelIndex src = mapToSource(index);
    if (!src.isValid()) {
        return {};
    }

    QVariant value = QIdentityProxyModel::data(index, role);

    bool isCard = src.data(DeckRoles::IsCardRole).toBool();

    if (role == Qt::FontRole && !isCard) {
        // Family comes from the DeckList theme role; bold stays caller-side.
        QFont f = themeManager->appFont(AppFont::DeckList);
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
