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

    QSignalSpy dirtySpy(&display, &CardDatabaseDisplayModel::modelDirty);
    display.setCardName("Do");
    ASSERT_TRUE(dirtySpy.wait());
    EXPECT_GT(display.rowCount(), 0);
    EXPECT_LT(display.rowCount(), source.rowCount());
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
