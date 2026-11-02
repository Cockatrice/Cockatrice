#include "gtest/gtest.h"
#include <QMetaType>
#include <QString>
#include <QVariantList>
#include <libcockatrice/game/board/card_state.h>
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

TEST(CardStateTest, CountersAddUpdateAndClear)
{
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Cat")), 1);
    int changes = 0;
    QObject::connect(&card, &CardState::countersChanged, [&changes]() { ++changes; });

    card.insertCounter(1, 2);
    EXPECT_EQ(card.getCounters().value(1), 2);

    card.setCounter(1, 3);
    EXPECT_EQ(card.getCounters().value(1), 3);

    card.setCounter(1, 0);
    EXPECT_FALSE(card.getCounters().contains(1));

    card.setCounter(2, 1);
    card.clearCounters();
    EXPECT_TRUE(card.getCounters().isEmpty());

    EXPECT_EQ(changes, 5);
}

TEST(CardStateTest, TapFlipAttackingAndReset)
{
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Cat")), 1);
    card.setTapped(true);
    card.setFaceDown(true);
    card.setAttacking(true);
    card.setAnnotation(QStringLiteral("note"));
    card.setPT(QStringLiteral("4/4"));
    card.setDoesntUntap(true);
    card.setDestroyOnZoneChange(true);
    card.setColor(QStringLiteral("red"));
    card.insertCounter(1, 2);

    card.resetState(true);

    EXPECT_FALSE(card.getTapped());
    EXPECT_FALSE(card.getAttacking());
    EXPECT_TRUE(card.getCounters().isEmpty());
    EXPECT_TRUE(card.getPT().isEmpty());
    EXPECT_FALSE(card.getDoesntUntap());
    EXPECT_EQ(card.getAnnotation(), QStringLiteral("note"));
    // resetState() deliberately leaves these three alone.
    EXPECT_TRUE(card.getFaceDown());
    EXPECT_EQ(card.getColor(), QStringLiteral("red"));
    EXPECT_TRUE(card.getDestroyOnZoneChange());

    card.setAnnotation(QStringLiteral("other"));
    card.resetState(false);
    EXPECT_TRUE(card.getAnnotation().isEmpty());
}

TEST(CardStateTest, ParsePT)
{
    const QVariantList plain = CardState::parsePT(QStringLiteral("3/3"));
    ASSERT_EQ(plain.size(), 2);
    EXPECT_EQ(plain.at(0).toString(), QStringLiteral("3"));
    EXPECT_EQ(plain.at(1).toString(), QStringLiteral("3"));

    const QVariantList negative = CardState::parsePT(QStringLiteral("-1/2"));
    ASSERT_EQ(negative.size(), 2);
    EXPECT_EQ(negative.at(0).typeId(), QMetaType::Int);
    EXPECT_EQ(negative.at(0).toInt(), -1);
    EXPECT_EQ(negative.at(1).toString(), QStringLiteral("2"));

    const QVariantList star = CardState::parsePT(QStringLiteral("*"));
    ASSERT_EQ(star.size(), 1);
    EXPECT_EQ(star.at(0).toString(), QStringLiteral("*"));

    EXPECT_TRUE(CardState::parsePT(QString()).isEmpty());
}
