#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <gtest/gtest.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <libcockatrice/models/deck_list/deck_list_sort_filter_proxy_model.h>

namespace
{
TEST(CollatorSort, NumericNamesSortNaturally)
{
    QStandardItemModel source(0, 2);
    const QStringList names = {"Forest 10", "Forest 2", "Forest 1"};
    for (int row = 0; row < names.size(); ++row) {
        source.setItem(row, DeckListModelColumns::CARD_NAME, new QStandardItem(names.at(row)));
    }

    DeckListSortFilterProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setSortCriteria({"name"});
    proxy.sort(DeckListModelColumns::CARD_NAME, Qt::AscendingOrder);

    QStringList actual;
    for (int row = 0; row < proxy.rowCount(); ++row) {
        actual.append(proxy.index(row, DeckListModelColumns::CARD_NAME).data(Qt::DisplayRole).toString());
    }

    EXPECT_EQ(actual, (QStringList{"Forest 1", "Forest 2", "Forest 10"}));
}
} // namespace
