#include "mocks.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

#define QUERY(name, card, query, match)                                                                                \
    TEST_F(CardQuery, name)                                                                                            \
    {                                                                                                                  \
        ASSERT_EQ(FilterString(query).check(card), match);                                                             \
    }

namespace
{

class CardQuery : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CardDatabase *db = new CardDatabase(nullptr, new NoopCardPreferenceProvider(),
                                            new TestCardDatabasePathProvider(), new NoopCardSetPriorityController());
        db->loadCardDatabases();

        cat = db->query()->getCardBySimpleName("Cat");
        notDeadAfterAll = db->query()->getCardBySimpleName("Not Dead");
        truth = db->query()->getCardBySimpleName("Truth");
        doctor = db->query()->getCardBySimpleName("Doctor");
    }
    // void TearDown() override {}

    CardData cat;
    CardData notDeadAfterAll;
    CardData truth;
    CardData doctor;
};

QUERY(Empty, cat, "", true)
QUERY(Typing, cat, "t", true)

QUERY(NonMatchingType, cat, "t:kithkin", false)
QUERY(MatchingType, cat, "t:creature", true)
QUERY(MatchingCreatureType, cat, "t:cat", true)
QUERY(PartialMatchingType, cat, "t:ca", false)
QUERY(MatchingMultiWordType, doctor, "t:\"Time Lord\"", true)
QUERY(Not1, cat, "NOT t:kithkin", true)
QUERY(Not2, cat, "NOT t:creature", false)
QUERY(NonKeyword1, cat, "not t:kithkin", false)
QUERY(NonKeyword2, cat, "t:bat or t:creature", false)
QUERY(NonKeyword3, notDeadAfterAll, "not dead", true)
QUERY(NonKeyword4, truth, "truth or trail", false)
QUERY(Case, cat, "t:cReAtUrE", true)

QUERY(And, cat, "t:creature t:creature", true)
QUERY(And2, cat, "t:creature t:sorcery", false)

QUERY(Or, cat, "t:bat OR t:creature", true)

QUERY(Cmc1, cat, "cmc=2", true)
QUERY(Cmc2, cat, "cmc>3", false)
QUERY(Cmc3, cat, "cmc>1", true)

QUERY(Quotes, cat, "t:\"creature\"", true)

QUERY(Field, cat, "pt:\"3/3\"", true)

QUERY(Color1, cat, "c:g", true)
QUERY(Color2, cat, "c:gw", true)
QUERY(Color3, cat, "c!g", true)
QUERY(Color4, cat, "c!gw", false)

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
