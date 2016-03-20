#include "gtest/gtest.h"

#include "carddatabase_test.h"

void CardDatabaseSettings::setSortKey(QString /* shortName */, unsigned int /* sortKey */) { };
void CardDatabaseSettings::setEnabled(QString /* shortName */, bool /* enabled */) { };
void CardDatabaseSettings::setIsKnown(QString /* shortName */, bool /* isknown */) { };
unsigned int CardDatabaseSettings::getSortKey(QString /* shortName */) { return 0; };
bool CardDatabaseSettings::isEnabled(QString /* shortName */) { return true; };
bool CardDatabaseSettings::isKnown(QString /* shortName */) { return true; };

SettingsCache::SettingsCache() { cardDatabaseSettings = new CardDatabaseSettings(); };
SettingsCache::~SettingsCache() { delete cardDatabaseSettings; };
QString SettingsCache::getCustomCardDatabasePath() const { return QString("%1/customsets/").arg(CARDDB_DATADIR); }
QString SettingsCache::getCardDatabasePath() const { return QString("%1/cards.xml").arg(CARDDB_DATADIR); }
QString SettingsCache::getTokenDatabasePath() const { return QString("%1/tokens.xml").arg(CARDDB_DATADIR); }
CardDatabaseSettings& SettingsCache::cardDatabase() const { return *cardDatabaseSettings; }

SettingsCache *settingsCache;

void PictureLoader::clearPixmapCache(CardInfo * /* card */) { }

// include out main header file _after_ the hack is complete
#include "../../cockatrice/src/carddatabase.h"

namespace {

    TEST(CardDatabaseTest, LoadXml) {
		settingsCache = new SettingsCache;
		CardDatabase *db = new CardDatabase;

        // ensure the card database is empty at start
        ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty at start";
        ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty at start";
        ASSERT_EQ(0, db->getAllColors().size()) << "Colors not empty at start";
        ASSERT_EQ(0, db->getAllMainCardTypes().size()) << "Types not empty at start";
        ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status at start";

        // load dummy cards and test result
        db->loadCardDatabases();
        ASSERT_EQ(6, db->getCardList().size()) << "Wrong card count after load";
        ASSERT_EQ(3, db->getSetList().size()) << "Wrong sets count after load";
        ASSERT_EQ(4, db->getAllColors().size()) << "Wrong colors count after load";
        ASSERT_EQ(2, db->getAllMainCardTypes().size()) << "Wrong types count after load";
        ASSERT_EQ(Ok, db->getLoadStatus()) << "Wrong status after load";

        // ensure the card database is empty after clear()
        db->clear();
        ASSERT_EQ(0, db->getCardList().size()) << "Cards not empty after clear";
        ASSERT_EQ(0, db->getSetList().size()) << "Sets not empty after clear";
        ASSERT_EQ(0, db->getAllColors().size()) << "Colors not empty after clear";
        ASSERT_EQ(0, db->getAllMainCardTypes().size()) << "Types not empty after clear";
        ASSERT_EQ(NotLoaded, db->getLoadStatus()) << "Incorrect status after clear";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}