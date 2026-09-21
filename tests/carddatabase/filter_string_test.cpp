#include "mocks.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/filters/filter_tree.h>
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

QUERY(BracketNextToUnquotedString, cat, "(o:woof OR o:meow)", true)

CardInfoPtr localizedCat()
{
    CardInfoPtr localized = CardInfo::newInstance("Cat", "Meow!", false, {}, {}, {}, {}, {});
    localized->setLocalizedName("de", "Kater");
    localized->setLocalizedText("de", "miaut");
    return localized;
}

TEST_F(CardQuery, SearchLanguageEnglishMatchesOnlyEnglish)
{
    const CardData localized = localizedCat();
    ASSERT_TRUE(FilterString("Cat", CardSearchLanguage{"de", SearchLanguageMode::English}).check(localized));
    ASSERT_FALSE(FilterString("Kater", CardSearchLanguage{"de", SearchLanguageMode::English}).check(localized));
}

TEST_F(CardQuery, SearchLanguageSelectedMatchesLocalizedNameAndText)
{
    const CardData localized = localizedCat();
    ASSERT_TRUE(FilterString("Kater", CardSearchLanguage{"de", SearchLanguageMode::Selected}).check(localized));
    ASSERT_TRUE(FilterString("o:miaut", CardSearchLanguage{"de", SearchLanguageMode::Selected}).check(localized));
    ASSERT_FALSE(FilterString("Cat", CardSearchLanguage{"de", SearchLanguageMode::Selected}).check(localized));
}

TEST_F(CardQuery, SearchLanguageSelectedFallsBackToEnglishForUntranslatedCards)
{
    const CardData localized = localizedCat();
    ASSERT_TRUE(FilterString("Cat", CardSearchLanguage{"fr", SearchLanguageMode::Selected}).check(localized));
    ASSERT_FALSE(FilterString("Kater", CardSearchLanguage{"fr", SearchLanguageMode::Selected}).check(localized));
}

TEST_F(CardQuery, SearchLanguageBothMatchesEitherLanguage)
{
    const CardData localized = localizedCat();
    ASSERT_TRUE(FilterString("Cat", CardSearchLanguage{"de", SearchLanguageMode::Both}).check(localized));
    ASSERT_TRUE(FilterString("Kater", CardSearchLanguage{"de", SearchLanguageMode::Both}).check(localized));
}

TEST_F(CardQuery, SearchLanguageIsBoundPerInstance)
{
    const CardData localized = localizedCat();

    FilterString germanQuery("Kater", CardSearchLanguage{"de", SearchLanguageMode::Selected});
    ASSERT_TRUE(germanQuery.check(localized));

    // Constructing an English-bound instance afterwards must not change the
    // language the earlier instance searches in.
    FilterString englishQuery("Kater", CardSearchLanguage{"", SearchLanguageMode::English});
    ASSERT_FALSE(englishQuery.check(localized));
    ASSERT_TRUE(germanQuery.check(localized));
}

CardInfoPtr taggedCard()
{
    return CardInfo::newInstance("Tagged Card", "text", false, {{"tags", "ramp removal"}}, {}, {}, {}, {});
}

TEST_F(CardQuery, TagsMatchWholeSlugs)
{
    const CardData tagged = taggedCard();
    ASSERT_TRUE(FilterString("tags:ramp").check(tagged));
    ASSERT_TRUE(FilterString("tags:removal").check(tagged));
    ASSERT_TRUE(FilterString("tags:RAMP").check(tagged));
    ASSERT_TRUE(FilterString("tag:ramp").check(tagged));
    ASSERT_FALSE(FilterString("tags:squirrel").check(tagged));
}

TEST_F(CardQuery, TagQueryDoesNotMatchPartialSlugs)
{
    const CardData tagged = taggedCard();
    ASSERT_FALSE(FilterString("tags:ram").check(tagged));
    ASSERT_FALSE(FilterString("tags:mov").check(tagged));
}

TEST_F(CardQuery, TagQueryCombinesWithAnd)
{
    const CardData tagged = taggedCard();
    ASSERT_TRUE(FilterString("tags:ramp tags:removal").check(tagged));
    ASSERT_FALSE(FilterString("tags:ramp tags:squirrel").check(tagged));
}

TEST_F(CardQuery, TagQueryTreatsCommasAsPartOfTheSlug)
{
    // Tag lists are not a thing: `tags:draw` and `tags:ramp` are separate terms.
    const CardData tagged = taggedCard();
    ASSERT_FALSE(FilterString("tags:ramp,removal").check(tagged));
}

TEST_F(CardQuery, FilterTreeTagAttribute)
{
    const CardData tagged = taggedCard();

    FilterTree matching;
    matching.termNode(CardFilter::AttrTag, CardFilter::TypeAnd, "ramp");
    ASSERT_TRUE(matching.acceptsCard(tagged, CardSearchLanguage{}));

    FilterTree partial;
    partial.termNode(CardFilter::AttrTag, CardFilter::TypeAnd, "ram");
    ASSERT_FALSE(partial.acceptsCard(tagged, CardSearchLanguage{}));

    FilterTree missing;
    missing.termNode(CardFilter::AttrTag, CardFilter::TypeAnd, "squirrel");
    ASSERT_FALSE(missing.acceptsCard(tagged, CardSearchLanguage{}));
}

TEST_F(CardQuery, TagQueryFalseWhenCardHasNoTags)
{
    ASSERT_FALSE(FilterString("tags:ramp").check(cat));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
