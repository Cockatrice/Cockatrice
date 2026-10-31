#include "gtest/gtest.h"
#include <QSignalSpy>
#include <QString>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/zones/hand_zone_logic.h>
#include <libcockatrice/utility/card_ref.h>

namespace
{

CardRef cardRefFor(const QString &name)
{
    CardRef cardRef;
    cardRef.name = name;
    return cardRef;
}

} // namespace

TEST(CardZoneLogicTest, AddsAndTakesCardsWithoutAGraphicsSide)
{
    HandZoneLogic zone(nullptr, QStringLiteral("hand"), false, false, true);
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Lightning Bolt")), 7);

    QSignalSpy cardAddedSpy(&zone, &CardZoneLogic::cardAdded);
    QSignalSpy cardCountSpy(&zone, &CardZoneLogic::cardCountChanged);

    zone.addCard(&card, false, 0);

    ASSERT_EQ(zone.getCards().size(), 1);
    EXPECT_EQ(zone.getCards().first(), &card);
    EXPECT_EQ(card.getZone(), &zone);
    EXPECT_EQ(cardAddedSpy.count(), 1);
    EXPECT_GE(cardCountSpy.count(), 1);

    EXPECT_EQ(zone.takeCard(0, 7), &card);
    EXPECT_TRUE(zone.getCards().isEmpty());
    EXPECT_EQ(card.getId(), 7);
}

TEST(CardZoneLogicTest, AsksTheGraphicsSideToCreateCards)
{
    HandZoneLogic zone(nullptr, QStringLiteral("library"), false, true, true);
    QSignalSpy requestSpy(&zone, &CardZoneLogic::createCardRequested);

    ServerInfo_Card cardInfo;
    cardInfo.set_name("Lightning Bolt");
    cardInfo.set_id(7);
    cardInfo.set_x(0);
    cardInfo.set_y(1);

    zone.createCardRequested(cardInfo, true);

    ASSERT_EQ(requestSpy.count(), 1);
    const QList<QVariant> arguments = requestSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<ServerInfo_Card>().id(), 7);
    EXPECT_TRUE(arguments.at(1).toBool());
}

TEST(CardZoneLogicTest, MoveBetweenZonesUpdatesCardZone)
{
    HandZoneLogic first(nullptr, QStringLiteral("hand"), false, false, true);
    HandZoneLogic second(nullptr, QStringLiteral("grave"), false, false, true);
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Cat")), 7);

    first.addCard(&card, false, 0);
    ASSERT_EQ(first.getCards().size(), 1);

    CardState *taken = first.takeCard(0, 7);
    ASSERT_EQ(taken, &card);
    EXPECT_TRUE(first.getCards().isEmpty());

    second.addCard(taken, false, 0);
    EXPECT_EQ(second.getCards().size(), 1);
    EXPECT_EQ(card.getZone(), &second);
}

TEST(CardZoneLogicTest, GetCardFindsById)
{
    HandZoneLogic zone(nullptr, QStringLiteral("hand"), false, false, true);
    CardState first(nullptr, nullptr, cardRefFor(QStringLiteral("Cat")), 7);
    CardState second(nullptr, nullptr, cardRefFor(QStringLiteral("Dog")), 8);

    zone.addCard(&first, false, 0);
    zone.addCard(&second, false, 0);

    EXPECT_EQ(zone.getCard(7), &first);
    EXPECT_EQ(zone.getCard(8), &second);
    EXPECT_EQ(zone.getCard(99), nullptr);
}

TEST(CardZoneLogicTest, ReorganizeSignalFiresOnAdd)
{
    HandZoneLogic zone(nullptr, QStringLiteral("hand"), false, false, true);
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Cat")), 7);
    QSignalSpy reorganizeSpy(&zone, &CardZoneLogic::reorganizeCards);

    zone.addCard(&card, true, 0);

    EXPECT_EQ(reorganizeSpy.count(), 1);
}
