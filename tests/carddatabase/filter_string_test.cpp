#include "../../cockatrice/src/filter_string.h"
#include "mocks.h"

#include "gtest/gtest.h"
#include <cmath>

CardDatabase *db;

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
        cat = db->getCardBySimpleName("Cat");
    }

    // void TearDown() override {}

    CardData cat;
};

QUERY(Empty, cat, "", true)
QUERY(Typing, cat, "t", true)

QUERY(NonMatchingType, cat, "t:kithkin", false)
QUERY(MatchingType, cat, "t:creature", true)
QUERY(Not1, cat, "not t:kithkin", true)
QUERY(Not2, cat, "not t:creature", false)
QUERY(Case, cat, "t:cReAtUrE", true)

QUERY(And, cat, "t:creature t:creature", true)
QUERY(And2, cat, "t:creature t:sorcery", false)

QUERY(Or, cat, "t:bat or t:creature", true)

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
    settingsCache = new SettingsCache;
    db = new CardDatabase;
    db->loadCardDatabases();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
