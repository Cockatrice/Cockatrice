#include "../../oracle/src/parsehelpers.h"

#include "gtest/gtest.h"

TEST(ParseCiptTest, parsesThisEntersTapped)
{
    auto name = "Boring Fields";
    auto text = "This land enters tapped.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesThisEntersTheBattlefieldTapped)
{
    auto name = "Boring Fields";
    auto text = "This land enters the battlefield tapped.\n"
                "{T}: Add {G}.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesItEntersTappedAtEndOfSentence)
{
    auto name = "Shocking Fields";
    auto text = "As this land enters, you may pay 2 life. If you don't, it enters tapped.\n"
                "{T}: Add {G}.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesThisEntersTappedWhenNotOnFirstLine)
{
    auto name = "Boring Fields";
    auto text = "Flying\n"
                "This land enters tapped.\n"
                "{T}: Add {G}.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesFullNameWithUnderscoreAppendedText)
{
    auto name = "Boring Fields_SL50";
    auto text = "Boring Fields enters tapped.\n"
                "{T}: Add {G}.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesFullNameWithBracketsAppendedText)
{
    auto name = "Boring Fields (SL50)";
    auto text = "Boring Fields enters tapped.\n"
                "{T}: Add {G}.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesFullNameWithComma)
{
    auto name = "Bob, the Legend";
    auto text = "Bob, the Legend enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesFullNameWithCommaAtEndOfSentence)
{
    auto name = "Bob, the Legend";
    auto text = "As Bob, the Legend enters, you may pay 2 life. If you don't, Bob, the Legend enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesFullNameWithApostropheAtEndOfSentence)
{
    auto name = "Bob's Bobber";
    auto text = "As Bob's Bobber enters, you may pay 2 life. If you don't, Bob's Bobber enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesShortnameEndingWithComma)
{
    auto name = "Bob, the Legend";
    auto text = "Bob enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesShortnameEndingWithSpace)
{
    auto name = "Bob the Legend";
    auto text = "Bob enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesMultiWordShortnameEndingWithComma)
{
    auto name = "Bob Dod, the Legend";
    auto text = "Bob Dod enters tapped.\n"
                "Whenever Bob Dod attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesMultiWordShortnameEndingWithSpace)
{
    auto name = "Bob Dod the Legend";
    auto text = "Bob Dod enters tapped.\n"
                "Whenever Bob Dod attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesShortnameEndingWithSpaceWithUnderscoreAppendedText)
{
    auto name = "Bob the Legend_SL50";
    auto text = "Bob enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesShortnameEndingWithSpaceWithBracketsAppendedText)
{
    auto name = "Bob the Legend (SL50)";
    auto text = "Bob enters tapped.\n"
                "Whenever Bob attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesMultiWordShortnameEndingWithSpaceWithUnderscoreAppendedText)
{
    auto name = "Bob Dod the Legend_SL50";
    auto text = "Bob Dod enters tapped.\n"
                "Whenever Bob Dod attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesMultiWordShortnameEndingWithSpaceWithBracketsAppendedText)
{
    auto name = "Bob Dod the Legend (SL50)";
    auto text = "Bob Dod enters tapped.\n"
                "Whenever Bob Dod attacks, you win the game.";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsEmptyText)
{
    auto name = "Vanilla Dude";
    auto text = "";

    ASSERT_FALSE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsEntersTappedUnless)
{
    auto name = "Fast Fields";
    auto text = "This land enters tapped unless you control another land.";

    ASSERT_FALSE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsWhenNameIsDifferent)
{
    auto name = "Boring Fields";
    auto text = "Fast Fields enters tapped.";

    ASSERT_FALSE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsOtherCreaturesEnterTapped)
{
    auto name = "Imposing Guy";
    auto text = "Other creatures enter tapped.";

    ASSERT_FALSE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsAbilityGrantingEntersTapped)
{
    auto name = "Imposing Guy";
    auto text = "Other creatures have \"This creature enters tapped\".";

    ASSERT_FALSE(parseCipt(name, text));
}

TEST(ParseCiptTest, parsesEntersTappedAndAbilityGrantingEntersTappedOnSameCard)
{
    auto name = "Imposing Guy";
    auto text = "This creature enters tapped."
                "Other creatures have \"This creature enters tapped\".";

    ASSERT_TRUE(parseCipt(name, text));
}

TEST(ParseCiptTest, rejectsItEntersTappedAndAttacking)
{
    auto name = "Token Maker";
    auto text = "When Token Maker attacks, create a token. It enters tapped and attacking.";

    ASSERT_FALSE(parseCipt(name, text));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
