/**
 * @file game_smoke_test.cpp
 *
 * Links the game logic library into a target that contains no graphics code at all, and drives
 * the two seams the graphics layer used to own: the attachment graph on CardState and the
 * card-creation request a zone makes so the graphics side can build the matching card item.
 */

#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/zones/hand_zone_logic.h>

namespace
{

CardRef cardRefFor(const QString &name)
{
    CardRef cardRef;
    cardRef.name = name;
    return cardRef;
}

} // namespace

TEST(CardStateTest, AttachmentGraphKeepsBothEndsConsistent)
{
    CardState host(nullptr, nullptr, cardRefFor(QStringLiteral("Host")), 1);
    CardState guest(nullptr, nullptr, cardRefFor(QStringLiteral("Guest")), 2);

    guest.setAttachedTo(&host);
    host.addAttachedCard(&guest);

    EXPECT_EQ(guest.getAttachedTo(), &host);
    ASSERT_EQ(host.getAttachedCards().size(), 1);
    EXPECT_EQ(host.getAttachedCards().first(), &guest);

    host.removeAttachedCard(&guest);
    guest.setAttachedTo(nullptr);

    EXPECT_TRUE(host.getAttachedCards().isEmpty());
    EXPECT_EQ(guest.getAttachedTo(), nullptr);
}

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
    QSignalSpy requestSpy(&zone, &CardZoneLogic::requestCreateCard);

    ServerInfo_Card cardInfo;
    cardInfo.set_name("Lightning Bolt");
    cardInfo.set_id(7);
    cardInfo.set_x(0);
    cardInfo.set_y(1);

    zone.requestCreateCard(cardInfo, true);

    ASSERT_EQ(requestSpy.count(), 1);
    const QList<QVariant> arguments = requestSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<ServerInfo_Card>().id(), 7);
    EXPECT_TRUE(arguments.at(1).toBool());
}

int main(int argc, char **argv)
{
    // Keep SettingsCache reads and writes away from the real user profile: the game logic still
    // reaches for a handful of settings, and the client settings sources linked into this test
    // would otherwise touch the developer's own configuration.
    QStandardPaths::setTestModeEnabled(true);

    QTemporaryDir home;
    if (home.isValid()) {
        qputenv("HOME", home.path().toLocal8Bit());
    }

    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules("settings_cache.*=false");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
