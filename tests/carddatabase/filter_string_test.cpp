#include "gtest/gtest.h"
#include "../../cockatrice/src/filter_string.h"
#include "mocks.h"

#include <cmath>

CardDatabase *db;

#define Query(name, card, query, match) \
TEST_F(CardQuery, name) {\
    ASSERT_EQ(FilterString(query).check(card), match);\
}


namespace
{

class CardQuery : public ::testing::Test {
 protected:
  void SetUp() override {
     cat = db->getCardBySimpleName("Cat");
  }

  // void TearDown() override {}

  CardData cat;
};

    Query(Empty, cat, "", true)
    Query(Typing, cat, "t", true)

    Query(NonMatchingType, cat, "t:kithkin", false)
    Query(MatchingType, cat, "t:creature", true)
    Query(Not1, cat, "not t:kithkin", true)
    Query(Not2, cat, "not t:creature", false)
    Query(Case, cat, "t:cReAtUrE", true)

    Query(And, cat, "t:creature t:creature", true)
    Query(And2, cat, "t:creature t:sorcery", false)
    
    Query(Or, cat, "t:bat or t:creature", true)

    Query(Cmc1, cat, "cmc=2", true)
    Query(Cmc2, cat, "cmc>3", false)
    Query(Cmc3, cat, "cmc>1", true)

    Query(Quotes, cat, "t:\"creature\"", true);

    Query(Field, cat, "pt:\"3/3\"", true)

    Query(Color1, cat, "c:g", true);
    Query(Color2, cat, "c:gw", true);
    Query(Color3, cat, "c!g", true);
    Query(Color4, cat, "c!gw", false);

} // namespace

int main(int argc, char **argv)
{
    settingsCache = new SettingsCache;
    db = new CardDatabase;
    db->loadCardDatabases();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}