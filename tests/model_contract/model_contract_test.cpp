#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"
#include "libcockatrice/models/database/card_database_display_model.h"
#include "libcockatrice/models/database/card_database_model.h"
#include "libcockatrice/models/database/card_set/card_sets_model.h"
#include "libcockatrice/models/deck_list/deck_list_model.h"
#include "model_contract_checker.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QModelIndex>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QVariant>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/game_specific_terms.h>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <libcockatrice/settings/download_settings.h>

namespace
{
QSharedPointer<DeckList> makeDeck()
{
    return QSharedPointer<DeckList>(new DeckList());
}

void loadCardDatabase(CardDatabase &database)
{
    database.loadCardDatabases();
    ASSERT_EQ(database.getLoadStatus(), Ok);
}

TEST(ModelContract, DeckListModelKeepsContractWhileZonesAndCardsChange)
{
    const QSharedPointer<DeckList> deck = makeDeck();
    DeckListModel model(nullptr, deck);

    test_support::ModelContractChecker checker(&model);

    ASSERT_NE(deck->getTree()->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    model.rebuildTree();

    CardInfoPtr bolt = CardInfo::newInstance("Lightning Bolt");
    const QModelIndex added = model.addCard(ExactCard(bolt), DECK_ZONE_MAIN);
    ASSERT_TRUE(added.isValid());

    CardInfoPtr bears = CardInfo::newInstance("Grizzly Bears");
    bears->setProperty(Mtg::MainCardType, "Creature");
    const QModelIndex grouped = model.addCard(ExactCard(bears), "Removal");
    ASSERT_TRUE(grouped.isValid());

    ASSERT_TRUE(model.removeRows(grouped.row(), 1, grouped.parent()));
}

TEST(ModelContract, DeckListModelEmitsCardAddedSignal)
{
    const QSharedPointer<DeckList> deck = makeDeck();
    DeckListModel model(nullptr, deck);
    QSignalSpy spy(&model, &DeckListModel::cardAddedAt);

    const QModelIndex added = model.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);

    ASSERT_TRUE(added.isValid());
    EXPECT_EQ(spy.count(), 1);
}

TEST(ModelContract, CardDatabaseModelAndDisplayProxyKeepContract)
{
    CardDatabase database(nullptr, new NoopCardPreferenceProvider(), new TestCardDatabasePathProvider(),
                          new NoopCardSetPriorityController());
    ASSERT_NO_FATAL_FAILURE(loadCardDatabase(database));

    CardDatabaseModel source(&database, false);
    test_support::ModelContractChecker sourceChecker(&source);

    CardDatabaseDisplayModel display;
    display.setSourceModel(&source);
    test_support::ModelContractChecker displayChecker(&display);

    ASSERT_GT(source.rowCount(), 0);
    EXPECT_EQ(display.rowCount(), source.rowCount());

    // A card row is a leaf, so its index must not claim child columns. The
    // tester does not check this, hence the explicit assertion.
    const QModelIndex sourceCard = source.index(0, 0);
    ASSERT_TRUE(sourceCard.isValid());
    EXPECT_EQ(source.columnCount(sourceCard), 0);
    const QModelIndex displayCard = display.index(0, 0);
    ASSERT_TRUE(displayCard.isValid());
    EXPECT_EQ(display.columnCount(displayCard), 0);

    QSignalSpy dirtySpy(&display, &CardDatabaseDisplayModel::modelDirty);
    display.setCardName("Do");
    ASSERT_TRUE(dirtySpy.wait());
    EXPECT_GT(display.rowCount(), 0);
    EXPECT_LT(display.rowCount(), source.rowCount());
}

TEST(ModelContract, ContinuumCardStatsAreSortableAndLabeled)
{
    NoopCardPreferenceProvider prefs;
    TestCardDatabasePathProvider paths;
    NoopCardSetPriorityController setPriority;
    CardDatabase database(nullptr, &prefs, &paths, &setPriority);

    auto swift = CardInfo::newInstance("Swift");
    swift->setProperty("identity", "Astra");
    swift->setProperty("attack", "9");
    swift->setProperty("armour", "10");
    swift->setProperty("health", "2");

    auto sturdy = CardInfo::newInstance("Sturdy");
    sturdy->setProperty("identity", "Zephyr");
    sturdy->setProperty("attack", "10");
    sturdy->setProperty("armour", "2");
    sturdy->setProperty("health", "12");

    database.addCard(swift);
    database.addCard(sturdy);

    CardDatabaseModel source(&database, false);
    CardDatabaseDisplayModel display;
    display.setSourceModel(&source);

    ASSERT_EQ(source.columnCount(), CardDatabaseModel::ColumnCount);
    ASSERT_EQ(display.rowCount(), 2);
    EXPECT_EQ(source.headerData(CardDatabaseModel::AtkHpColumn, Qt::Horizontal).toString(), "ATK/HP");
    EXPECT_EQ(source.headerData(CardDatabaseModel::IdentityColumn, Qt::Horizontal).toString(), "Identity");
    EXPECT_EQ(source.headerData(CardDatabaseModel::AttackColumn, Qt::Horizontal).toString(), "Attack");
    EXPECT_EQ(source.headerData(CardDatabaseModel::ArmourColumn, Qt::Horizontal).toString(), "Armour");
    EXPECT_EQ(source.headerData(CardDatabaseModel::HealthColumn, Qt::Horizontal).toString(), "Health");

    const auto firstName = [&display] { return display.index(0, CardDatabaseModel::NameColumn).data().toString(); };

    display.sort(CardDatabaseModel::AttackColumn, Qt::AscendingOrder);
    EXPECT_EQ(firstName(), "Swift") << "9 ATK should precede 10 ATK";
    display.sort(CardDatabaseModel::ArmourColumn, Qt::AscendingOrder);
    EXPECT_EQ(firstName(), "Sturdy") << "2 Armour should precede 10 Armour";
    display.sort(CardDatabaseModel::HealthColumn, Qt::AscendingOrder);
    EXPECT_EQ(firstName(), "Swift") << "2 HP should precede 12 HP";
    display.sort(CardDatabaseModel::AtkHpColumn, Qt::AscendingOrder);
    EXPECT_EQ(firstName(), "Swift") << "ATK/HP should sort attack and health numerically";
    display.sort(CardDatabaseModel::IdentityColumn, Qt::DescendingOrder);
    EXPECT_EQ(firstName(), "Sturdy") << "Identity should sort text from the alias/identity property";
}

TEST(ModelContract, SetsModelKeepsContract)
{
    CardDatabase database(nullptr, new NoopCardPreferenceProvider(), new TestCardDatabasePathProvider(),
                          new NoopCardSetPriorityController());
    ASSERT_NO_FATAL_FAILURE(loadCardDatabase(database));

    SetsModel model(&database);
    test_support::ModelContractChecker checker(&model);

    ASSERT_GT(model.rowCount(), 0);
    ASSERT_GT(model.columnCount(), 0);
}

TEST(ModelContract, SettingsSignalIsObservableWithQSignalSpy)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    DownloadSettings settings(tempDir.path() + "/", nullptr);
    QSignalSpy spy(&settings, &DownloadSettings::downloadSpoilerStatusChanged);

    settings.setDownloadSpoilerStatus(true);

    ASSERT_EQ(spy.count(), 1);
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
