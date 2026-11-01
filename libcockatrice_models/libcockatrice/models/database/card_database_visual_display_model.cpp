#include "card_database_visual_display_model.h"

#include "card_database_display_model.h"
#include "libcockatrice/card/database/card_database_querier.h"

#include <QAbstractItemModel>
#include <QList>
#include <QStringList>
#include <QVariant>

/// Coalesces the bursts of proxy signals a filter, sort or database load produces.
static constexpr int REBUILD_COALESCE_MS = 30;

CardDatabaseVisualDisplayModel::CardDatabaseVisualDisplayModel(CardDatabaseDisplayModel *_displayModel,
                                                               CardDatabaseQuerier *_querier,
                                                               QObject *parent)
    : QAbstractListModel(parent), displayModel(_displayModel), querier(_querier)
{
    rebuildTimer.setSingleShot(true);
    rebuildTimer.setInterval(REBUILD_COALESCE_MS);
    connect(&rebuildTimer, &QTimer::timeout, this, &CardDatabaseVisualDisplayModel::rebuild);

    if (displayModel == nullptr) {
        return;
    }

    connect(displayModel, &QAbstractItemModel::modelReset, this, &CardDatabaseVisualDisplayModel::scheduleRebuild);
    connect(displayModel, &QAbstractItemModel::layoutChanged, this, &CardDatabaseVisualDisplayModel::scheduleRebuild);
    connect(displayModel, &QAbstractItemModel::rowsInserted, this, &CardDatabaseVisualDisplayModel::scheduleRebuild);
    connect(displayModel, &QAbstractItemModel::rowsRemoved, this, &CardDatabaseVisualDisplayModel::scheduleRebuild);
    connect(displayModel, &QAbstractItemModel::dataChanged, this, &CardDatabaseVisualDisplayModel::scheduleRebuild);
}

void CardDatabaseVisualDisplayModel::setSetFilterTerms(const QStringList &terms)
{
    if (setFilterTerms == terms) {
        return;
    }

    setFilterTerms = terms;
    refresh();
}

void CardDatabaseVisualDisplayModel::refresh()
{
    rebuildTimer.stop();
    rebuild();
}

void CardDatabaseVisualDisplayModel::scheduleRebuild()
{
    rebuildTimer.start();
}

void CardDatabaseVisualDisplayModel::rebuild()
{
    beginResetModel();
    entries.clear();

    if (displayModel != nullptr) {
        const int proxyRows = displayModel->rowCount();
        entries.reserve(proxyRows);
        for (int row = 0; row < proxyRows; ++row) {
            const CardInfoPtr info = displayModel->cardInfoAt(displayModel->index(row, 0));
            if (info) {
                appendPrintingsFor(info);
            }
        }
    }

    endResetModel();
}

void CardDatabaseVisualDisplayModel::appendPrintingsFor(const CardInfoPtr &info)
{
    if (setFilterTerms.isEmpty()) {
        entries.append({info, PrintingInfo()});
        return;
    }

    const auto &printingsBySet = info->getSets();
    for (const QString &term : setFilterTerms) {
        const auto printings = printingsBySet.constFind(term);
        if (printings == printingsBySet.constEnd()) {
            continue;
        }
        for (const PrintingInfo &printing : printings.value()) {
            entries.append({info, printing});
        }
    }
}

int CardDatabaseVisualDisplayModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(entries.size());
}

QVariant CardDatabaseVisualDisplayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= entries.size()) {
        return {};
    }

    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        return cardAt(index.row()).getName();
    }

    return {};
}

ExactCard CardDatabaseVisualDisplayModel::cardAt(int row) const
{
    if (row < 0 || row >= entries.size()) {
        return {};
    }

    const Entry &entry = entries.at(row);
    if (entry.printing.isEmpty() && setFilterTerms.isEmpty()) {
        return querier != nullptr ? querier->getPreferredCard(entry.info) : ExactCard(entry.info);
    }

    return ExactCard(entry.info, entry.printing);
}
