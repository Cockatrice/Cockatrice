#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/set/card_set.h"
#include "libcockatrice/card/set/card_set_list.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QHash>
#include <QIODevice>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVector>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_data.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/card/lazy_properties_hash.h>
#include <libcockatrice/card/printing/printing_info.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <string>

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

/**
 * @brief Path provider serving a sandboxed copy of the test card database.
 */
class SandboxPathProvider : public ICardDatabasePathProvider
{
public:
    explicit SandboxPathProvider(const QString &_root) : root(_root)
    {
    }

    QString getCardDatabasePath() const override
    {
        return root + "/cards.xml";
    }

    QString getCustomCardDatabasePath() const override
    {
        return root + "/customsets/";
    }

    QString getTokenDatabasePath() const override
    {
        return root + "/tokens.xml";
    }

    QString getSpoilerCardDatabasePath() const override
    {
        return root + "/spoiler.xml";
    }

private:
    QString root;
};

/**
 * @brief Set priority controller whose enablement state the test drives directly.
 *
 * Seeded short names behave like entries present in cardDatabase.ini (enabled,
 * known). Anything the parser discovers but the test did not seed falls back to
 * the default SetOptions, i.e. disabled and unknown, mirroring
 * CardDatabaseSettings for sets without an ini group.
 */
class SeededSetPriorityController : public ICardSetPriorityController
{
public:
    void seed(const QStringList &shortNames)
    {
        for (const QString &shortName : shortNames) {
            SetOptions options;
            options.enabled = true;
            options.isKnown = true;
            setOptions.insert(shortName, options);
        }
    }

    void setSortKey(QString shortName, unsigned int sortKey) override
    {
        setOptions[shortName].sortKey = sortKey;
    }

    void setEnabled(QString shortName, bool enabled) override
    {
        setOptions[shortName].enabled = enabled;
    }

    void setIsKnown(QString shortName, bool isknown) override
    {
        setOptions[shortName].isKnown = isknown;
    }

    unsigned int getSortKey(QString shortName) const override
    {
        return setOptions.value(shortName).sortKey;
    }

    bool isEnabled(QString shortName) const override
    {
        return setOptions.value(shortName).enabled;
    }

    bool isKnown(QString shortName) const override
    {
        return setOptions.value(shortName).isKnown;
    }

    SetOptions getSetOptions(QString shortName) const override
    {
        return setOptions.value(shortName);
    }

    QStringList getEnabledSetNames() const override
    {
        QStringList names;
        for (auto it = setOptions.constBegin(); it != setOptions.constEnd(); ++it) {
            if (it.value().enabled) {
                names << it.key();
            }
        }
        names.sort();
        return names;
    }

    void saveSets(const QVector<SetSaveData> &data) override
    {
        for (const SetSaveData &entry : data) {
            setOptions[entry.shortName].sortKey = entry.sortKey;
            setOptions[entry.shortName].enabled = entry.enabled;
        }
    }

private:
    QHash<QString, SetOptions> setOptions;
};

/**
 * @brief Sandboxed card database plus enablement controller for cache tests.
 *
 * Copies the checked-in fixture XML into a temporary directory so each test
 * owns the files (and the binary cache written next to them).
 */
class CardDatabaseCacheInvalidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tempDir.isValid());
        root = tempDir.path();
        ASSERT_TRUE(QFile::copy(QString(CARDDB_DATADIR) + "cards.xml", root + "/cards.xml"));
        ASSERT_TRUE(QFile::copy(QString(CARDDB_DATADIR) + "tokens.xml", root + "/tokens.xml"));
        ASSERT_TRUE(QDir(root).mkpath("customsets"));
        ASSERT_TRUE(
            QFile::copy(QString(CARDDB_DATADIR) + "customsets/customset1.xml", root + "/customsets/customset1.xml"));
        controller.seed({"CAT", "DOG", "WHO", " Not a Card", "BRD"});
    }

    /**
     * @brief Contents of the binary cache written next to cards.xml.
     * @return The cache file bytes, or an empty array if no cache exists.
     */
    QByteArray cacheContents() const
    {
        QFile file(root + "/cards.xml.cache");
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }
        return file.readAll();
    }

    QTemporaryDir tempDir;
    QString root;
    SeededSetPriorityController controller;
};

TEST_F(CardDatabaseCacheInvalidationTest, DisablingSetBustsCacheAndDropsPrintings)
{
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    const QByteArray initialCache = cacheContents();
    ASSERT_FALSE(initialCache.isEmpty()) << "first load must write the binary cache";
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    ASSERT_TRUE(db.getCardList().value("Cat")->getSets().contains("CAT"));

    // A cache-aware load with unchanged inputs keeps the cache: the parse
    // result would be identical, so there is nothing to invalidate.
    db.loadCardDatabases();
    EXPECT_EQ(initialCache, cacheContents()) << "load with unchanged inputs must keep the cache";

    // Disabling a set changes the source hash, so the next load re-parses and
    // the set's printings disappear everywhere.
    controller.setEnabled("CAT", false);
    db.reloadCardDatabasesAndNotify();
    EXPECT_NE(initialCache, cacheContents()) << "toggling set enablement must bust the cache";
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    EXPECT_FALSE(db.getCardList().value("Cat")->getSets().contains("CAT")) << "disabled set printings must be dropped";
    ASSERT_FALSE(db.getCardList().value("Kitten").isNull());
    EXPECT_FALSE(db.getCardList().value("Kitten")->getSets().contains("CAT"));
}

TEST_F(CardDatabaseCacheInvalidationTest, AddingAndEnablingCustomSetBustsCache)
{
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    const QByteArray initialCache = cacheContents();
    ASSERT_FALSE(initialCache.isEmpty()) << "first load must write the binary cache";

    // Drop a new custom set in: the input file list changes, so the next load
    // must re-parse. BRD2 is unknown, so its set starts disabled.
    QFile customSetFile(root + "/customsets/customset2.xml");
    ASSERT_TRUE(customSetFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    customSetFile.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<cockatrice_carddatabase version=\"4\">\n"
                        "    <cards>\n"
                        "        <card>\n"
                        "            <name>Finch</name>\n"
                        "            <set>BRD2</set>\n"
                        "            <tablerow>0</tablerow>\n"
                        "            <text>Chirp!</text>\n"
                        "        </card>\n"
                        "    </cards>\n"
                        "</cockatrice_carddatabase>\n");
    customSetFile.close();

    db.reloadCardDatabasesAndNotify();
    const QByteArray afterCustomSetAdded = cacheContents();
    EXPECT_NE(initialCache, afterCustomSetAdded) << "adding a custom card database must bust the cache";
    ASSERT_FALSE(db.getCardList().value("Finch").isNull()) << "the custom card must be loaded";
    EXPECT_TRUE(db.getCardList().value("Finch")->getSets().isEmpty())
        << "BRD2 starts disabled, so Finch has no printings yet";

    // Enabling the freshly discovered set busts the cache again, so the next
    // load finally parses its printings (#7455).
    controller.setEnabled("BRD2", true);
    db.reloadCardDatabasesAndNotify();
    EXPECT_NE(afterCustomSetAdded, cacheContents()) << "enabling a set must bust the cache";
    ASSERT_FALSE(db.getCardList().value("Finch").isNull());
    EXPECT_TRUE(db.getCardList().value("Finch")->getSets().contains("BRD2"));
}

TEST_F(CardDatabaseCacheInvalidationTest, ExplicitReloadBypassesCache)
{
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    ASSERT_TRUE(db.getCardList().value("Cat")->getSets().contains("CAT"));

    // Move Cat to the DOG set in place, keeping the file size identical and
    // restoring the modification time afterwards: the source hash (path, size,
    // mtime) still matches, so the cache stays valid by construction and only
    // an explicit reload can pick the edit up.
    const QString cardsPath = root + "/cards.xml";
    QFile file(cardsPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    const qint64 stampedSecs = QFileInfo(cardsPath).lastModified().toSecsSinceEpoch();
    QByteArray contents = file.readAll();
    file.close();
    const QByteArray oldSetCode("<set>CAT</set>");
    const QByteArray newSetCode("<set>DOG</set>");
    ASSERT_EQ(contents.count(oldSetCode), 1) << "fixture must contain exactly one CAT printing";
    contents.replace(contents.indexOf(oldSetCode), oldSetCode.size(), newSetCode);
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    ASSERT_EQ(file.write(contents), contents.size());
    // Flush the pending write before stamping the time: close() would otherwise
    // flush it afterwards and bump the modification time again, invalidating the
    // cache on platforms that buffer writes (e.g. macOS).
    ASSERT_TRUE(file.flush());
    ASSERT_TRUE(file.setFileTime(QDateTime::fromSecsSinceEpoch(stampedSecs), QFileDevice::FileModificationTime));
    file.close();
    ASSERT_EQ(QFileInfo(cardsPath).lastModified().toSecsSinceEpoch(), stampedSecs)
        << "the sandbox file's modification time must be restored for the cache to stay valid";

    // The cache is still valid for a regular load, which serves the stale data.
    db.loadCardDatabases();
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    EXPECT_TRUE(db.getCardList().value("Cat")->getSets().contains("CAT")) << "regular load must hit the cache";
    EXPECT_FALSE(db.getCardList().value("Cat")->getSets().contains("DOG"));

    // The explicit reload ignores the cache and re-reads the file.
    const QByteArray staleCache = cacheContents();
    db.reloadCardDatabasesAndNotify();
    EXPECT_NE(staleCache, cacheContents()) << "explicit reload must rewrite the cache from disk";
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    EXPECT_FALSE(db.getCardList().value("Cat")->getSets().contains("CAT"));
    EXPECT_TRUE(db.getCardList().value("Cat")->getSets().contains("DOG"));
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}