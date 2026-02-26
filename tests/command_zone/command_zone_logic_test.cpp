/**
 * @file command_zone_logic_test.cpp
 * @brief Unit tests for CommandZoneLogic::addCardImpl
 *
 * Tests the card insertion logic for command zones:
 * - Position handling (negative x, beyond range, valid x)
 * - Card visibility and state reset
 * - Unknown contents handling (sets card ID to -1)
 */

#include "add_card_algorithm.h"
#include "mocks/mock_card_item.h"

#include <QList>
#include <gtest/gtest.h>

/**
 * @brief Standalone CardList implementation for testing.
 *
 * Mimics the behavior of the real CardList class without dependencies.
 */
class TestCardList : public QList<MockCardItem *>
{
    bool contentsKnown;

public:
    explicit TestCardList(bool _contentsKnown = true) : contentsKnown(_contentsKnown)
    {
    }

    [[nodiscard]] bool getContentsKnown() const
    {
        return contentsKnown;
    }

    void setContentsKnown(bool known)
    {
        contentsKnown = known;
    }
};

/**
 * @brief Testable implementation of CommandZoneLogic::addCardImpl.
 *
 * Uses the shared CardZoneAlgorithms::addCardToList template to ensure
 * test behavior matches production code exactly.
 */
class CommandZoneLogicTestHarness
{
public:
    TestCardList cards;

    explicit CommandZoneLogicTestHarness(bool contentsKnown = true) : cards(contentsKnown)
    {
    }

    /**
     * @brief Delegates to shared algorithm for testing.
     *
     * Uses CardZoneAlgorithms::addCardToList which is the same template
     * used by CommandZoneLogic::addCardImpl in production code.
     */
    void addCardImpl(MockCardItem *card, int x, int /*y*/)
    {
        CardZoneAlgorithms::addCardToList(cards, card, x);
    }
};

class CommandZoneLogicTest : public ::testing::Test
{
protected:
    CommandZoneLogicTestHarness *harness;

    void SetUp() override
    {
        harness = new CommandZoneLogicTestHarness(true);
    }

    void TearDown() override
    {
        qDeleteAll(harness->cards);
        harness->cards.clear();
        delete harness;
    }

    MockCardItem *createCard(int id = 1)
    {
        return new MockCardItem(id);
    }

    void addExistingCards(int count)
    {
        for (int i = 0; i < count; ++i) {
            harness->cards.append(createCard(100 + i));
        }
    }
};

// Test: Negative x appends card to end of list
TEST_F(CommandZoneLogicTest, AddCard_NegativeX_AppendsToEnd)
{
    addExistingCards(2); // Cards at indices 0, 1
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, -1, 0);

    ASSERT_EQ(3, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(2)) << "Card should be at end (index 2)";
}

// Test: x beyond range appends card to end
TEST_F(CommandZoneLogicTest, AddCard_BeyondRange_AppendsToEnd)
{
    addExistingCards(2); // Cards at indices 0, 1
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 100, 0); // x=100 is way beyond size=2

    ASSERT_EQ(3, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(2)) << "Card should be at end (index 2)";
}

// Test: Valid x inserts card at that position
TEST_F(CommandZoneLogicTest, AddCard_ValidX_InsertsAtPosition)
{
    addExistingCards(3); // Cards at indices 0, 1, 2
    MockCardItem *cardAtIndex1 = harness->cards.at(1);
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 1, 0);

    ASSERT_EQ(4, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(1)) << "New card should be at index 1";
    EXPECT_EQ(cardAtIndex1, harness->cards.at(2)) << "Original card at index 1 should shift to index 2";
}

// Test: y parameter is ignored
TEST_F(CommandZoneLogicTest, AddCard_YIsIgnored)
{
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 0, 999); // y=999 should be ignored

    ASSERT_EQ(1, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(0)) << "Card should be at index 0 regardless of y value";
}

// Test: Card is set to visible after adding
TEST_F(CommandZoneLogicTest, AddCard_SetsVisibleTrue)
{
    MockCardItem *newCard = createCard(42);
    ASSERT_FALSE(newCard->visible) << "Card should start invisible";

    harness->addCardImpl(newCard, 0, 0);

    EXPECT_TRUE(newCard->visible) << "Card should be visible after adding";
}

// Test: resetState is called with true
TEST_F(CommandZoneLogicTest, AddCard_CallsResetState)
{
    MockCardItem *newCard = createCard(42);
    ASSERT_FALSE(newCard->resetStateCalled) << "resetState should not be called initially";

    harness->addCardImpl(newCard, 0, 0);

    EXPECT_TRUE(newCard->resetStateCalled) << "resetState should be called";
    EXPECT_TRUE(newCard->resetStateShowFaceDown) << "resetState should be called with true";
}

// Test: When contents unknown, card ID is set to -1
TEST_F(CommandZoneLogicTest, AddCard_UnknownContents_SetsIdNegative)
{
    delete harness;
    harness = new CommandZoneLogicTestHarness(false); // contentsKnown = false

    MockCardItem *newCard = createCard(42);
    ASSERT_EQ(42, newCard->getId()) << "Card should start with original ID";

    harness->addCardImpl(newCard, 0, 0);

    EXPECT_EQ(-1, newCard->getId()) << "Card ID should be -1 when contents unknown";
}

// Test: When contents known, card ID is preserved
TEST_F(CommandZoneLogicTest, AddCard_KnownContents_PreservesId)
{
    MockCardItem *newCard = createCard(42);
    ASSERT_EQ(42, newCard->getId()) << "Card should start with original ID";

    harness->addCardImpl(newCard, 0, 0);

    EXPECT_EQ(42, newCard->getId()) << "Card ID should be preserved when contents known";
}

// Test: Insert at position 0 (beginning)
TEST_F(CommandZoneLogicTest, AddCard_AtZero_InsertsAtBeginning)
{
    addExistingCards(2);
    MockCardItem *originalFirst = harness->cards.at(0);
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 0, 0);

    ASSERT_EQ(3, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(0)) << "New card should be at beginning";
    EXPECT_EQ(originalFirst, harness->cards.at(1)) << "Original first card should shift to index 1";
}

// Test: Insert into empty list
TEST_F(CommandZoneLogicTest, AddCard_EmptyList_AppendsCard)
{
    ASSERT_EQ(0, harness->cards.size()) << "List should start empty";
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 0, 0);

    ASSERT_EQ(1, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(0));
}

// Test: x equal to size appends (boundary condition)
TEST_F(CommandZoneLogicTest, AddCard_XEqualToSize_AppendsToEnd)
{
    addExistingCards(2); // size = 2
    MockCardItem *newCard = createCard(42);

    harness->addCardImpl(newCard, 2, 0); // x == size, should append

    ASSERT_EQ(3, harness->cards.size());
    EXPECT_EQ(newCard, harness->cards.at(2)) << "Card should be at end";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
