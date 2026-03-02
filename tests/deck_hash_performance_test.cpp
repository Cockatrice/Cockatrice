#include "gtest/gtest.h"
#include <QDebug>
#include <libcockatrice/deck_list/deck_list.h>

static constexpr int amount = 1e5;
QString repeatDeck;
QString numberDeck;
QString uniquesDeck;
QString uniquesXorDeck;
QString duplicatesDeck;

TEST(DeckHashTest, RepeatTest)
{
    DeckList decklist(repeatDeck);
    for (int i = 0; i < amount; ++i) {
        decklist.getDeckHash();
        decklist.refreshDeckHash();
    }
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "5cac19qm") << "The hash does not match!";
}

TEST(DeckHashTest, NumberTest)
{
    DeckList decklist(numberDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "e0m38p19") << "The hash does not match!";
}

TEST(DeckHashTest, UniquesTest)
{
    DeckList decklist(uniquesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "88prk025") << "The hash does not match!";
}

TEST(DeckHashTest, UniquesTestXor)
{
    DeckList decklist(uniquesXorDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "hkn6q4pf") << "The hash does not match!";
}

TEST(DeckHashTest, DuplicatesTest)
{
    DeckList decklist(duplicatesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "ekt6tg1h") << "The hash does not match!";
}

int main(int argc, char **argv)
{
    const QString deckStart =
        R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main">)";
    const QString deckEnd = R"(</zone></cockatrice_deck>)";

    repeatDeck =
        deckStart +
        R"(<card number="1" name="Mountain"/><card number="2" name="Island"/></zone><zone name="side"><card number="3" name="Forest"/>)" +
        deckEnd;
    numberDeck = deckStart + QString(R"(<card number="%1" name="Island"/>)").arg(amount) + deckEnd;

    QStringList deckString{deckStart};
    QStringList deckStringXor = deckString;
    int len = QString::number(amount).length();
    for (int i = 0; i < amount; ++i) {
        // creates already sorted list
        deckString << R"(<card number="1" name="card )" << QString::number(i).rightJustified(len, '0') << R"("/>)";
        // xor in order to mess with sorting
        deckStringXor << R"(<card number="1" name="card )" << QString::number(i ^ amount) << R"("/>)";
    }
    deckString << deckEnd;
    deckStringXor << deckEnd;
    uniquesDeck = deckString.join("");
    uniquesXorDeck = deckStringXor.join("");

    duplicatesDeck = deckStart + QString(R"(<card number="1" name="card"/>)").repeated(amount) + deckEnd;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
