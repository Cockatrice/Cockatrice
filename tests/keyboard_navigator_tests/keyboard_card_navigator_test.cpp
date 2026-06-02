#include "game/keyboard_card_navigator.h"

#include <QApplication>
#include <QKeyEvent>
#include <QList>
#include <QMap>
#include <QMetaObject>
#include <gtest/gtest.h>

// Some tests require us to get the zones out of a player, so instead of changing code we didn't make, we
// decided to use this dirty trick.
#define private public
#include "game/player/player_logic.h"
#undef private

#include "game/board/abstract_card_item.h"
#include "game/board/arrow_item.h"
#include "game/board/card_item.h"
#include "game/board/card_list.h"
#include "game/keyboard_card_navigator.cpp"
#include "game/zones/card_zone_logic.h"
#include "game/zones/hand_zone_logic.h"
#include "game/zones/stack_zone_logic.h"
#include "game/zones/table_zone_logic.h"
#include "keyboard_navigator_test_fakes.h"

class KeyboardCardNavigatorTest : public ::testing::Test
{
protected:
    KeyboardCardNavigator *navigator;
    FakeHandZoneLogic *handZone;
    FakeTableZoneLogic *tableZone;
    FakeStackZoneLogic *stackZone;
    PlayerLogic *player;

    void SetUp() override
    {
        handZone = new FakeHandZoneLogic("hand");
        tableZone = new FakeTableZoneLogic("table");
        stackZone = new FakeStackZoneLogic("stack");
        // Player is mocked like this, so we fill the zones for the tests.
        player = (PlayerLogic *)malloc(sizeof(PlayerLogic));
        new (&player->zones) QMap<QString, CardZoneLogic *>();
        player->zones.insert("table", tableZone);
        player->zones.insert("stack", stackZone);
        player->zones.insert("hand", handZone);
        navigator = new KeyboardCardNavigator(player);
    }

    void TearDown() override
    {
        delete navigator;
        free(player);
        delete handZone;
        delete tableZone;
        delete stackZone;
    }
};

/*  This test verifies the behaviour of spawning the cursor:
    When pressing the left arrow, it goes to the first card.
    When pressing the right arrow, it goes to the last card of the zone. */
TEST_F(KeyboardCardNavigatorTest, LeftRightArrowTest)
{
    // Set an arbitrary amount of cards for the zone
    handZone->setDummyCardCount(5);

    // Set the default index
    navigator->setHoveredCardIndex(-1);

    navigator->setCurrentZone(handZone);

    // Simulate a key press and a card switch
    QKeyEvent eRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    navigator->switchCardInZone(&eRight);

    // Verify the first card is selected
    EXPECT_EQ(navigator->getHoveredIndex(), 0);

    // Reset the index
    navigator->setHoveredCardIndex(-1);

    // Simulate a key press and a card switch
    QKeyEvent eLeft(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    navigator->switchCardInZone(&eLeft);

    // Check if the last card was selected
    EXPECT_EQ(navigator->getHoveredIndex(), 4);
}

/*  This test verifies the moving behaviour of the cursor. */
TEST_F(KeyboardCardNavigatorTest, NormalSwitchCards)
{
    // Set an arbitrary amount of cards for the zone
    handZone->setDummyCardCount(5);
    navigator->setCurrentZone(handZone);
    // Select the second card
    navigator->setHoveredCardIndex(1);

    // If right is pressed, go to index + 1
    QKeyEvent eRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    navigator->switchCardInZone(&eRight);
    EXPECT_EQ(navigator->getHoveredIndex(), 2);

    // If left is pressed, go to index - 1
    QKeyEvent eLeft(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    navigator->switchCardInZone(&eLeft);
    EXPECT_EQ(navigator->getHoveredIndex(), 1);
}

/*  This test verifies the edge case moving behaviour of the cursor.
    If we are on the first card, and left is pressed, we should go to the
    last card, and vice-versa */
TEST_F(KeyboardCardNavigatorTest, ZoneLoopsTest)
{
    // Set an arbitrary amount of cards for the zone
    tableZone->setDummyCardCount(2);

    // Select the first card
    navigator->setCurrentZone(tableZone);
    navigator->setHoveredCardIndex(0);

    // If we press left, it wraps around to the end, and the zone doesn't change
    QKeyEvent eLeft(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    navigator->switchCardInZone(&eLeft);
    EXPECT_EQ(navigator->getHoveredIndex(), 1);
    EXPECT_EQ(navigator->getCurrentZone(), tableZone);

    // If we press right, it wraps around to the start, and the zone doesn't change
    QKeyEvent eRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    navigator->switchCardInZone(&eRight);
    EXPECT_EQ(navigator->getHoveredIndex(), 0);
    EXPECT_EQ(navigator->getCurrentZone(), tableZone);
}

/* This test verifies the switching of zones if the zone we are currently on is empty. */
TEST_F(KeyboardCardNavigatorTest, EmptyZoneLoopTest)
{
    QList<CardZoneLogic *> zonesList;
    zonesList.append(tableZone);
    zonesList.append(stackZone);
    zonesList.append(handZone);

    tableZone->setDummyCardCount(2);
    stackZone->setDummyCardCount(0); // empty
    handZone->setDummyCardCount(2);

    // Simulate key up when switching zones
    CardZoneLogic *newZone = navigator->findZoneWithCards(zonesList, 0, true);
    // The result should be zone 2
    EXPECT_EQ(newZone, handZone);

    // Simulate key down when switching zones
    newZone = navigator->findZoneWithCards(zonesList, 2, false);
    // The result should be zone 2
    EXPECT_EQ(newZone, tableZone);
}

/* This test verifies the switching of zones with the up and down keys. */
TEST_F(KeyboardCardNavigatorTest, SwitchZoneTest)
{
    tableZone->setDummyCardCount(2);
    stackZone->setDummyCardCount(2);
    handZone->setDummyCardCount(2);

    navigator->setCurrentZone(tableZone);
    navigator->setHoveredCardIndex(0);

    // Simulate key up when switching zones
    QKeyEvent eUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    navigator->switchZone(&eUp);
    // The result should be zone 1 and the card selected is the first one
    EXPECT_EQ(navigator->getCurrentZone(), stackZone);
    EXPECT_EQ(navigator->getHoveredIndex(), 0);

    // Simulate key down when switching zones
    QKeyEvent eDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    navigator->switchZone(&eDown);
    // The result should be zone 0 and the card selected is the first one
    EXPECT_EQ(navigator->getCurrentZone(), tableZone);
    EXPECT_EQ(navigator->getHoveredIndex(), 0);
}

/* This test verifies the case where every zone is empty. */
TEST_F(KeyboardCardNavigatorTest, EmptyZoneSwitchZones)
{
    QList<CardZoneLogic *> zonesList;
    zonesList.append(tableZone);
    zonesList.append(stackZone);
    zonesList.append(handZone);

    tableZone->setDummyCardCount(0);
    stackZone->setDummyCardCount(0);
    handZone->setDummyCardCount(0);

    // The expected zone is the starting one
    CardZoneLogic *newZone = navigator->findZoneWithCards(zonesList, 0, true);
    EXPECT_EQ(newZone, tableZone);
}

/* This test verifies the behaviour when someone moves a card with the mouse and the
   keyboard is used after a zone becomes empty. */
TEST_F(KeyboardCardNavigatorTest, ZoneEmptySwitchTest)
{
    tableZone->setDummyCardCount(1);
    stackZone->setDummyCardCount(0);
    handZone->setDummyCardCount(1);

    // Set a card as hovered in the table zone
    navigator->setCurrentZone(tableZone);
    navigator->setHoveredCardIndex(0);

    // Simulate the user deleting/moving all cards out of the table zone
    tableZone->setDummyCardCount(0);

    // Now the user presses an arrow key in the empty zone
    QKeyEvent eRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    navigator->switchCardInZone(&eRight);

    // The expected zone is the next one with cards
    EXPECT_EQ(navigator->getCurrentZone(), handZone);
    EXPECT_EQ(navigator->getHoveredIndex(), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
