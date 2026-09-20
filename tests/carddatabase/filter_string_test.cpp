#include "mocks.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/printing/printing_info.h>
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

QUERY(SetCodeCaseInsensitive1, cat, "set:cat", true)
QUERY(SetCodeCaseInsensitive2, cat, "set:CAT", true)
QUERY(SetCodeCaseInsensitive3, cat, "set:CAt", true)
QUERY(SetCodeShortForm, cat, "e:cat", true)
QUERY(SetCodeWrongSet, cat, "set:who", false)
QUERY(SetCodeWrongSet2, doctor, "set:cat", false)

QUERY(BracketNextToUnquotedString, cat, "(o:woof OR o:meow)", true)

} // namespace

class SetQuery : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // EOE and EOC share a release date, like a set and its Commander counterpart.
        const QDate sharedReleaseDate(2026, 3, 13);
        mainSet = CardSet::newInstance(&controller, "EOE", "Edge of Eternities", "expansion", sharedReleaseDate,
                                       CardSet::PriorityPrimary);
        commanderSet = CardSet::newInstance(&controller, "EOC", "Edge of Eternities Commander", "commander",
                                            sharedReleaseDate, CardSet::PrioritySecondary);

        inBothSets = newCardWithPrintings({{"EOE", mainSet}, {"EOC", commanderSet}});
        onlyInCommanderSet = newCardWithPrintings({{"EOC", commanderSet}});
    }

    CardInfoPtr newCardWithPrintings(const QList<QPair<QString, CardSetPtr>> &printings)
    {
        SetToPrintingsMap setsInfo;
        for (const auto &printing : printings) {
            setsInfo[printing.first].append(PrintingInfo(printing.second));
        }
        return CardInfo::newInstance("Test Card", "", false, {}, {}, {}, setsInfo, CardInfo::UiAttributes());
    }

    NoopCardSetPriorityController controller;
    CardSetPtr mainSet;
    CardSetPtr commanderSet;
    CardInfoPtr inBothSets;
    CardInfoPtr onlyInCommanderSet;
};

TEST_F(SetQuery, ExactMatchSeparatesSameDaySets)
{
    EXPECT_TRUE(FilterString("set:EOE").check(inBothSets));
    EXPECT_TRUE(FilterString("e:EOE").check(inBothSets));
    EXPECT_TRUE(FilterString("set:EOC").check(inBothSets));
    EXPECT_FALSE(FilterString("set:EOE").check(onlyInCommanderSet));
    EXPECT_TRUE(FilterString("set:EOC").check(onlyInCommanderSet));
}

TEST_F(SetQuery, ExactMatchIsCaseInsensitive)
{
    EXPECT_TRUE(FilterString("set:eoe").check(inBothSets));
    EXPECT_TRUE(FilterString("set:EoE").check(inBothSets));
    EXPECT_FALSE(FilterString("set:eoe").check(onlyInCommanderSet));
}

TEST_F(SetQuery, NotEqualsMatchesPrintingsOutsideTheSet)
{
    EXPECT_FALSE(FilterString("set!EOE").check(inBothSets));
    EXPECT_TRUE(FilterString("set!EOE").check(onlyInCommanderSet));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
