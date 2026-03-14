#include "add_card_algorithm.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

struct MockCardRef
{
};

struct MockCard
{
    int idSet = 0;
    bool idWasCalled = false;
    MockCardRef cardRefSet{};
    bool cardRefWasCalled = false;
    bool resetStateCalled = false;
    bool resetStateKeepAnnotations = false;
    bool visibleSet = false;
    bool setVisibleCalled = false;

    void setId(int id)
    {
        idSet = id;
        idWasCalled = true;
    }

    void setCardRef(MockCardRef ref)
    {
        cardRefSet = ref;
        cardRefWasCalled = true;
    }

    void resetState(bool keepAnnotations)
    {
        resetStateCalled = true;
        resetStateKeepAnnotations = keepAnnotations;
    }

    void setVisible(bool visible)
    {
        setVisibleCalled = true;
        visibleSet = visible;
    }
};

class MockCardList
{
    std::vector<MockCard *> cards;
    bool contentsKnown;

public:
    explicit MockCardList(bool _contentsKnown) : contentsKnown(_contentsKnown)
    {
    }

    int size() const
    {
        return static_cast<int>(cards.size());
    }

    void insert(int index, MockCard *card)
    {
        cards.insert(cards.begin() + index, card);
    }

    bool getContentsKnown() const
    {
        return contentsKnown;
    }

    MockCard *at(int index) const
    {
        return cards.at(index);
    }

    void setContentsKnown(bool known)
    {
        contentsKnown = known;
    }
};

class AddCardAlgorithmTest : public ::testing::Test
{
protected:
    MockCardList knownList{true};
    MockCardList unknownList{false};
};

TEST_F(AddCardAlgorithmTest, InsertAtValidPosition)
{
    MockCard a, b, c;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &b, 1, false);
    CardZoneAlgorithms::addCardToList(knownList, &c, 1, false);

    EXPECT_EQ(knownList.at(0), &a);
    EXPECT_EQ(knownList.at(1), &c);
    EXPECT_EQ(knownList.at(2), &b);
}

TEST_F(AddCardAlgorithmTest, NegativeIndexClampsToEnd)
{
    MockCard a, b;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &b, -1, false);

    EXPECT_EQ(knownList.at(0), &a);
    EXPECT_EQ(knownList.at(1), &b);
    EXPECT_EQ(knownList.size(), 2);
}

TEST_F(AddCardAlgorithmTest, IndexBeyondSizeClampsToEnd)
{
    MockCard a, b;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &b, 999, false);

    EXPECT_EQ(knownList.at(0), &a);
    EXPECT_EQ(knownList.at(1), &b);
    EXPECT_EQ(knownList.size(), 2);
}

TEST_F(AddCardAlgorithmTest, IndexEqualToSizeAppends)
{
    MockCard a, b;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &b, 1, false);

    EXPECT_EQ(knownList.at(0), &a);
    EXPECT_EQ(knownList.at(1), &b);
}

TEST_F(AddCardAlgorithmTest, InsertIntoEmptyList)
{
    MockCard a;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);

    EXPECT_EQ(knownList.size(), 1);
    EXPECT_EQ(knownList.at(0), &a);
}

TEST_F(AddCardAlgorithmTest, ContentsKnownPreservesIdentity)
{
    MockCard card;
    CardZoneAlgorithms::addCardToList(knownList, &card, 0, false);

    EXPECT_FALSE(card.idWasCalled);
    EXPECT_FALSE(card.cardRefWasCalled);
}

TEST_F(AddCardAlgorithmTest, ContentsUnknownClearsIdentity)
{
    MockCard card;
    CardZoneAlgorithms::addCardToList(unknownList, &card, 0, false);

    EXPECT_TRUE(card.idWasCalled);
    EXPECT_EQ(card.idSet, -1);
    EXPECT_TRUE(card.cardRefWasCalled);
}

TEST_F(AddCardAlgorithmTest, KeepAnnotationsFalsePassedThrough)
{
    MockCard card;
    CardZoneAlgorithms::addCardToList(knownList, &card, 0, false);

    EXPECT_TRUE(card.resetStateCalled);
    EXPECT_FALSE(card.resetStateKeepAnnotations);
}

TEST_F(AddCardAlgorithmTest, KeepAnnotationsTruePassedThrough)
{
    MockCard card;
    CardZoneAlgorithms::addCardToList(knownList, &card, 0, true);

    EXPECT_TRUE(card.resetStateCalled);
    EXPECT_TRUE(card.resetStateKeepAnnotations);
}

TEST_F(AddCardAlgorithmTest, CardSetVisibleAfterAdd)
{
    MockCard card;
    CardZoneAlgorithms::addCardToList(knownList, &card, 0, false);

    EXPECT_TRUE(card.setVisibleCalled);
    EXPECT_TRUE(card.visibleSet);
}

TEST_F(AddCardAlgorithmTest, MultipleInsertionsMaintainCorrectOrder)
{
    MockCard a, b, c, d;
    CardZoneAlgorithms::addCardToList(knownList, &a, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &b, 0, false);
    CardZoneAlgorithms::addCardToList(knownList, &c, 1, false);
    CardZoneAlgorithms::addCardToList(knownList, &d, 3, false);

    EXPECT_EQ(knownList.size(), 4);
    EXPECT_EQ(knownList.at(0), &b);
    EXPECT_EQ(knownList.at(1), &c);
    EXPECT_EQ(knownList.at(2), &a);
    EXPECT_EQ(knownList.at(3), &d);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
