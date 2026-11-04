#include <gtest/gtest.h>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/essence_types.h>
#include <libcockatrice/utility/essence_counter.h>

#include <QHash>
#include <QString>

TEST(EssenceTypesTest, HasExactlySixteenCanonicalSymbols)
{
    EXPECT_EQ(Essence::symbols(), QStringLiteral("MSOGEAQCJBRLVTPN"));
    EXPECT_EQ(Essence::symbols().size(), 16);
    EXPECT_TRUE(Essence::isSymbol(QChar('N'))); // N is colorless.
    EXPECT_TRUE(Essence::isSymbol(QChar('J'))); // J is Jasper, between C and B.
    EXPECT_FALSE(Essence::isSymbol(QChar('W'))); // MTG white is not Essence.
    EXPECT_FALSE(Essence::isSymbol(QChar('U'))); // MTG blue is not Essence.
}

TEST(EssenceTypesTest, NetworkCounterNamesHaveStableRoundTrip)
{
    EXPECT_EQ(Essence::symbols(), EssenceCounter::symbols());
    int position = 1; // Counter id 0 is the existing life counter.
    for (const QChar code : EssenceCounter::symbols()) {
        const QString networkName = EssenceCounter::name(code);
        EXPECT_TRUE(networkName.startsWith(QStringLiteral("essence:")));
        EXPECT_EQ(EssenceCounter::codeFromName(networkName), code);
        EXPECT_TRUE(EssenceCounter::isEssenceCounter(networkName));
        ++position;
    }
    EXPECT_EQ(position, 17);
    EXPECT_TRUE(EssenceCounter::name(QChar('W')).isEmpty());
    EXPECT_FALSE(EssenceCounter::isEssenceCounter(QStringLiteral("essence:W")));
    EXPECT_FALSE(EssenceCounter::isEssenceCounter(QStringLiteral("essence:MM")));
    EXPECT_FALSE(EssenceCounter::isEssenceCounter(QStringLiteral("life")));
    EXPECT_EQ(EssenceCounter::clampBalance(-1), 0);
    EXPECT_EQ(EssenceCounter::clampBalance(-900), 0);
    EXPECT_EQ(EssenceCounter::clampBalance(0), 0);
    EXPECT_EQ(EssenceCounter::clampBalance(19), 19);
}

TEST(EssenceTypesTest, ParsesAllBracedAndUnbracedTypes)
{
    const QString raw = QStringLiteral("{M}{S}{O}{G}{E}{A}{Q}{C}{J}{B}{R}{L}{V}{T}{P}{N}");
    EXPECT_EQ(Essence::parseCost(raw).join(QString()), Essence::symbols());
    EXPECT_EQ(Essence::parseCost(QStringLiteral("MSN")).join(QString()), QStringLiteral("MSN"));
    EXPECT_EQ(Essence::parseCost(QStringLiteral("{C}{J}{B}{N}")).join(QString()), QStringLiteral("CJBN"));
    EXPECT_EQ(Essence::parseCost(QStringLiteral("{2}{N}{M}")).join(QStringLiteral("|")),
              QStringLiteral("2|N|M"));
}

TEST(EssenceTypesTest, RejectsUnknownLegacyAndHybridCodes)
{
    EXPECT_TRUE(Essence::parseCost(QStringLiteral("{W}{U}{Z}{M/N}")).isEmpty());
    EXPECT_EQ(Essence::parseCost(QStringLiteral("{N}")), QStringList{QStringLiteral("N")});
}

TEST(EssenceTypesTest, NormalizesUniqueIdentityInCanonicalOrder)
{
    EXPECT_EQ(Essence::normalizeIdentity(QStringLiteral("NSSMMO")), QStringLiteral("MSON"));
    EXPECT_EQ(Essence::normalizeIdentity(QStringLiteral("N")), QStringLiteral("N"));
    EXPECT_EQ(Essence::normalizeIdentity(QStringLiteral("BJNJC")), QStringLiteral("CJBN"));
    EXPECT_EQ(Essence::normalizeIdentity(QStringLiteral("WU")), QString());
}

TEST(EssenceTypesTest, NativePropertiesTakePrecedenceOverLegacyFallbacks)
{
    const QHash<QString, QString> props = {
        {QStringLiteral("essencecost"), QStringLiteral("{M}{N}")},
        {QStringLiteral("essence"), QStringLiteral("NM")},
        {QStringLiteral("essencevalue"), QStringLiteral("2")},
        {QStringLiteral("manacost"), QStringLiteral("{W}{U}")},
        {QStringLiteral("colors"), QStringLiteral("WU")},
        {QStringLiteral("cmc"), QStringLiteral("9")}
    };

    const CardInfoPtr card = CardInfo::newInstance(QStringLiteral("Essence Test"),
                                                   QString(), false, props, {}, {}, {}, {});
    ASSERT_TRUE(card);
    EXPECT_EQ(card->getManaCost(), QStringLiteral("{M}{N}"));
    EXPECT_EQ(card->getEssenceTypes(), QStringLiteral("MN"));
    EXPECT_EQ(card->getColors(), QStringLiteral("MN"));
    EXPECT_EQ(card->getCmc(), QStringLiteral("2"));
}
