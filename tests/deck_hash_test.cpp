#include "../../common/decklist.h"
#include<QDebug>

#include "gtest/gtest.h"

QString bigDeck;

TEST(DeckHashTest, RepeatTest)
{
    DeckList decklist(R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="1" name="Mountain"/><card number="2" name="Island"/></zone><zone name="side"><card number="3" name="Forest"/></zone></cockatrice_deck>)");
    for (int i = 0; i < 1e6; ++i){ // repeat hashing a million times
      decklist.updateDeckHash();
    }
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "5cac19qm") << "The hash matches";
}

TEST(DeckHashTest, NumberTest)
{
    DeckList decklist(R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="1000000" name="Island"/></zone></cockatrice_deck>)");
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "2k8cdi4s") << "The hash matches";
}

TEST(DeckHashTest, UniquesTest)
{
    DeckList decklist(bigDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "s544mlan") << "The hash matches";
}

int main(int argc, char **argv)
{
        QStringList deckString;
        deckString << R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="1" name="card 0)";
        for (int i = 1; i < 1e6; ++i){ // add a million unique cards
          deckString << R"("/><card number="1" name="card )" + QString::number(i);
        }
        deckString << R"("/></zone></cockatrice_deck>)";
        bigDeck = deckString.join("");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
