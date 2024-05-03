#include "../../common/decklist.h"

#include "gtest/gtest.h"

static constexpr int amount = 1e6;
QString uniquesDeck;
QString uniquesXorDeck;
QString duplicatesDeck;

TEST(DeckHashTest, RepeatTest)
{
    DeckList decklist(
        R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="1" name="Mountain"/><card number="2" name="Island"/></zone><zone name="side"><card number="3" name="Forest"/></zone></cockatrice_deck>)");
    for (int i = 0; i < amount; ++i) {
        decklist.updateDeckHash();
    }
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "5cac19qm") << "The hash matches";
}

TEST(DeckHashTest, NumberTest)
{
    DeckList decklist(
        QString(
            R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="%1" name="Island"/></zone></cockatrice_deck>)")
            .arg(amount));
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "2k8cdi4s") << "The hash matches";
}

TEST(DeckHashTest, UniquesTest)
{
    DeckList decklist(uniquesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "ha1ja7ut") << "The hash matches";
}

TEST(DeckHashTest, UniquesTestXor)
{
    DeckList decklist(uniquesXorDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "mdhdn0lv") << "The hash matches";
}

TEST(DeckHashTest, DuplicatesTest)
{
    DeckList decklist(duplicatesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "lker9j7p") << "The hash matches";
}

int main(int argc, char **argv)
{
    QStringList deckString;
    deckString
        << R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main"><card number="1" name="card 0)";
    QStringList deckStringXor = deckString;
    int len = QString::number(amount).length();
    for (int i = 1; i < amount; ++i) {
        deckString << R"("/><card number="1" name="card )" +
                          QString::number(i).rightJustified(len, '0'); // creates already sorted list
        deckStringXor << R"("/><card number="1" name="card )" +
                             QString::number(i ^ amount); // xor in order to mess with sorting
    }
    deckString << R"("/></zone></cockatrice_deck>)";
    deckStringXor << R"("/></zone></cockatrice_deck>)";
    uniquesDeck = deckString.join("");
    uniquesXorDeck = deckStringXor.join("");
    duplicatesDeck =
        R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main">)" +
        QString(R"(<card number="1" name="card"/>)").repeated(amount) + R"(</zone></cockatrice_deck>)";

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
