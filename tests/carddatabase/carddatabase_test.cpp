#include "mocks.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

namespace
{

TEST(CardDatabaseTest, LoadXml)
{
    CardDatabase *db = new CardDatabase(nullptr, new NoopCardPreferenceProvider(), new TestCardDatabasePathProvider(),
                                        new NoopCardSetPriorityController());

    // ensure the card database is empty at start
    ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty at start";
    ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty at start";
    ASSERT_EQ(0, db->query()->getAllMainCardTypes().size()) << "Types not empty at start";
    ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status at start";

    // load dummy cards and test result
    db->loadCardDatabases();
    ASSERT_EQ(9, db->getCardList().size()) << "Wrong card count after load";
    ASSERT_EQ(5, db->getSetList().size()) << "Wrong sets count after load";
    ASSERT_EQ(3, db->query()->getAllMainCardTypes().size()) << "Wrong types count after load";
    ASSERT_EQ(Ok, db->getLoadStatus()) << "Wrong status after load";

    // ensure the card database is empty after clear()
    db->clear();
    ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty after clear";
    ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty after clear";
    ASSERT_EQ(0, db->query()->getAllMainCardTypes().size()) << "Types not empty after clear";
    ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status after clear";
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}