#include "mocks.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <QTemporaryDir>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_data.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/card/lazy_properties_hash.h>
#include <libcockatrice/card/printing/printing_info.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
namespace
{

TEST(CardDatabaseTest, LoadXml)
{
    CardDatabase *db = new CardDatabase(nullptr, new NoopCardPreferenceProvider(), new TestCardDatabasePathProvider(),
                                        new NoopCardSetPriorityController());

    // ensure the card database is empty at start
    ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty at start";
    ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty at start";
    ASSERT_EQ(0, db->query()->getAllMainCardTypes().size()) << "Types not empty at start";
    ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status at start";

    // load dummy cards and test result
    db->loadCardDatabases();
    ASSERT_EQ(9, db->getCardList().size()) << "Wrong card count after load";
    ASSERT_EQ(5, db->getSetList().size()) << "Wrong sets count after load";
    ASSERT_EQ(3, db->query()->getAllMainCardTypes().size()) << "Wrong types count after load";
    ASSERT_EQ(Ok, db->getLoadStatus()) << "Wrong status after load";

    // ensure the card database is empty after clear()
    db->clear();
    ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty after clear";
    ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty after clear";
    ASSERT_EQ(0, db->query()->getAllMainCardTypes().size()) << "Types not empty after clear";
    ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status after clear";
}

TEST(CardDatabaseTest, Xml4LocalizedDataRoundTrip)
{
    NoopCardSetPriorityController controller;
    CardSetPtr set =
        CardSet::newInstance(&controller, "TST", "Test Set", "expansion", QDate(), CardSet::PriorityPrimary);

    QHash<QString, QString> props;
    props["manacost"] = "1R";
    PrintingInfo printing(set, LazyPropertiesHash(props));
    SetToPrintingsMap setsInfo;
    setsInfo["TST"].append(printing);

    CardInfo::UiAttributes attributes = {.tableRow = 1};
    CardInfoPtr card =
        CardInfo::newInstance("Lightning Bolt", "Deal 3 damage.", false, {}, {}, {}, setsInfo, attributes);
    card->setLocalizedName("de", "Blitzschlag");
    card->setLocalizedText("de", "Blitzschlag fügt 3 Schadenspunkte zu.");

    SetNameMap sets;
    sets.insert("TST", set);
    CardNameMap cards;
    cards.insert("Lightning Bolt", card);

    QTemporaryDir tempDir;
    const QString fileName = tempDir.filePath("cards.xml");
    NoopCardPreferenceProvider prefProvider;
    CockatriceXml4Parser writer(&prefProvider, &controller);
    ASSERT_TRUE(writer.saveToFile({}, sets, cards, fileName));

    CardDatabaseData data;
    CockatriceXml4Parser parser(&prefProvider, &controller);
    QFile file(fileName);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    parser.parseFileInto(file, data);

    CardInfoPtr loaded = data.cards.value("Lightning Bolt");
    ASSERT_FALSE(loaded.isNull());
    ASSERT_EQ(loaded->getName(), "Lightning Bolt");
    ASSERT_EQ(loaded->getLocalizedName("de"), "Blitzschlag");
    ASSERT_EQ(loaded->getLocalizedText("de"), "Blitzschlag fügt 3 Schadenspunkte zu.");
    ASSERT_EQ(loaded->getLocalizedText("fr"), "Deal 3 damage.");
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}