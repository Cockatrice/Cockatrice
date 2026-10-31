#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
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
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVector>
#include <QWeakPointer>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_data.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/card/lazy_properties_hash.h>
#include <libcockatrice/card/printing/printing_info.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <libcockatrice/utility/card_ref.h>
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

TEST(CardDatabaseTest, LazyPropertiesHashStreamsBlobAccessors)
{
    QHash<QString, QString> props;
    props.insert("manacost", "1R");
    props.insert("colors", "R");
    props.insert("format-standard", "legal");

    LazyPropertiesHash lazy = LazyPropertiesHash::serializedFrom(props);
    const QByteArray blob = lazy.serialized();

    EXPECT_EQ(3, lazy.size());
    EXPECT_FALSE(lazy.isEmpty());
    EXPECT_TRUE(lazy.contains("colors"));
    EXPECT_FALSE(lazy.contains("missing"));
    EXPECT_EQ("1R", lazy.value("manacost"));
    EXPECT_EQ(QString(), lazy.value("missing"));
    EXPECT_EQ(blob, lazy.serialized()) << "an untouched blob must be passed through unchanged";

    QStringList keys = lazy.keys();
    keys.sort();
    QStringList expectedKeys = props.keys();
    expectedKeys.sort();
    EXPECT_EQ(expectedKeys, keys);

    // Mutating materializes the hash, after which every accessor keeps working.
    EXPECT_TRUE(lazy.insert("loyalty", "3"));
    EXPECT_FALSE(lazy.insert("loyalty", "3"));
    EXPECT_EQ(4, lazy.size());
    EXPECT_EQ("3", lazy.value("loyalty"));
    EXPECT_TRUE(lazy.contains("loyalty"));

    LazyPropertiesHash roundTrip(lazy.serialized());
    EXPECT_EQ(4, roundTrip.size());
    EXPECT_EQ("3", roundTrip.value("loyalty"));
    EXPECT_EQ("legal", roundTrip.value("format-standard"));
}

TEST(CardDatabaseTest, GuessCardWithProviderIdOnUnknownCardReturnsEmpty)
{
    // An unknown name plus a providerId used to dereference the missing card
    // inside findPrintingWithId(); the lookup must come back empty instead.
    NoopCardPreferenceProvider prefs;
    TestCardDatabasePathProvider pathProvider;
    NoopCardSetPriorityController controller;
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    const ExactCard guessed = db.query()->guessCard({"Not a Card", "card_Not a Card_fixture"});

    EXPECT_TRUE(guessed.isEmpty());
    EXPECT_FALSE(guessed);
}

TEST(CardDatabaseTest, FindPrintingWithIdOnNullCardInfoReturnsEmpty)
{
    NoopCardPreferenceProvider prefs;
    TestCardDatabasePathProvider pathProvider;
    NoopCardSetPriorityController controller;
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    EXPECT_TRUE(db.query()->findPrintingWithId(nullptr, "card_Not a Card_fixture").isEmpty());
}

TEST(CardInfoTest, ClearDirectRelationsDropsRawPointers)
{
    auto *forward = new CardRelation("Child");
    auto *reverse = new CardRelation("Parent");
    auto *toMe = new CardRelation("Child");

    auto card = CardInfo::newInstance("Parent", "", false, QHash<QString, QString>(), {forward}, {reverse},
                                      SetToPrintingsMap(), CardInfo::UiAttributes());
    card->addReverseRelatedCards2Me(toMe);
    ASSERT_EQ(1, card->getRelatedCards().size());
    ASSERT_EQ(1, card->getReverseRelatedCards().size());
    ASSERT_EQ(1, card->getReverseRelatedCards2Me().size());
    ASSERT_EQ(2, card->getAllRelatedCards().size());

    card->clearDirectRelations();

    EXPECT_TRUE(card->getRelatedCards().isEmpty());
    EXPECT_TRUE(card->getReverseRelatedCards().isEmpty());
    EXPECT_TRUE(card->getReverseRelatedCards2Me().isEmpty());
    EXPECT_TRUE(card->getAllRelatedCards().isEmpty());

    delete forward;
    delete reverse;
    delete toMe;
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
    card->setProperty("format-standard", "legal");
    card->setProperty("colors", "R");

    SetNameMap sets;
    sets.insert("TST", set);
    CardNameMap cards;
    cards.insert("Lightning Bolt", card);

    QTemporaryDir tempDir;
    const QString fileName = tempDir.filePath("cards.xml");
    CockatriceXml4Parser writer(&controller);
    ASSERT_TRUE(writer.saveToFile({}, sets, cards, fileName));

    CardDatabaseData data;
    CockatriceXml4Parser parser(&controller);
    QFile file(fileName);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    parser.parseFileInto(file, data);

    CardInfoPtr loaded = data.cards.value("Lightning Bolt");
    ASSERT_FALSE(loaded.isNull());
    ASSERT_EQ(loaded->getName(), "Lightning Bolt");
    ASSERT_EQ(loaded->getLocalizedName("de"), "Blitzschlag");
    ASSERT_EQ(loaded->getLocalizedText("de"), "Blitzschlag fügt 3 Schadenspunkte zu.");
    ASSERT_EQ(loaded->getLocalizedText("fr"), "Deal 3 damage.");
    ASSERT_EQ(loaded->getProperty("format-standard"), "legal");
    ASSERT_TRUE(loaded->hasProperty("colors"));
    ASSERT_EQ(loaded->getProperties().size(), 2);

    const QList<PrintingInfo> printings = loaded->getSets().value("TST");
    ASSERT_FALSE(printings.isEmpty());
    ASSERT_EQ(printings.first().getProperty("manacost"), "1R");
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
        ++individualWrites;
        setOptions[shortName].sortKey = sortKey;
    }

    void setEnabled(QString shortName, bool enabled) override
    {
        ++individualWrites;
        setOptions[shortName].enabled = enabled;
    }

    void setIsKnown(QString shortName, bool isknown) override
    {
        ++individualWrites;
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
        ++batchedWrites;
        for (const SetSaveData &entry : data) {
            setOptions[entry.shortName].sortKey = entry.sortKey;
            setOptions[entry.shortName].enabled = entry.enabled;
            setOptions[entry.shortName].isKnown = entry.isKnown;
        }
    }

    int individualWrites = 0; ///< Per-set/per-option controller writes (the slow path).
    int batchedWrites = 0;    ///< Batched saveSets() flushes (the fast path).

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

TEST_F(CardDatabaseCacheInvalidationTest, FirstRunWithNoSetsEnabledSkipsCacheWrite)
{
    // A blank config: the parser discovers every set, but none of them has an
    // ini entry, so all of them default to disabled (first run).
    SeededSetPriorityController blankController;
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &blankController);

    db.loadCardDatabases();
    ASSERT_FALSE(db.getCardList().value("Cat").isNull()) << "cards are parsed even while their sets are disabled";
    EXPECT_TRUE(db.getCardList().value("Cat")->getSets().isEmpty())
        << "no printings survive an all-sets-disabled parse";
    EXPECT_TRUE(cacheContents().isEmpty()) << "a stripped first-run snapshot must not be cached";
}

TEST_F(CardDatabaseCacheInvalidationTest, EnablingAllSetsAfterFirstRunRestoresPrintings)
{
    SeededSetPriorityController blankController;
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &blankController);

    db.loadCardDatabases();
    ASSERT_TRUE(cacheContents().isEmpty());

    // This mirrors what checkUnknownSets() does on a first run: enable every
    // set, then reload so the parser keeps the printings and the complete
    // snapshot gets cached under the enabled-set hash.
    for (const CardSetPtr &set : db.getSetList()) {
        blankController.setEnabled(set->getShortName(), true);
    }
    db.reloadCardDatabasesAndNotify();

    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    EXPECT_TRUE(db.getCardList().value("Cat")->getSets().contains("CAT"));
    EXPECT_FALSE(cacheContents().isEmpty()) << "the enabled-set snapshot must be cached";
}

TEST_F(CardDatabaseCacheInvalidationTest, CheckUnknownSetsBatchesFirstRunEnableAll)
{
    // Same first-run starting point as above, but driven through the real
    // checkUnknownSets() path, which must persist the enable-all with a single
    // batched saveSets() instead of per-set/per-option controller writes.
    SeededSetPriorityController blankController;
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &blankController);

    db.loadCardDatabases();
    ASSERT_TRUE(db.getSetList().getEnabledSetsNum() == 0);

    db.checkUnknownSets();

    EXPECT_EQ(blankController.individualWrites, 0) << "first-run enable-all must not write per set";
    EXPECT_EQ(blankController.batchedWrites, 1) << "first-run enable-all must persist with a single flush";
    for (const CardSetPtr &set : db.getSetList()) {
        EXPECT_TRUE(set->getEnabled()) << set->getShortName().toStdString();
        EXPECT_TRUE(blankController.isEnabled(set->getShortName())) << set->getShortName().toStdString();
        if (!set->getIsKnownIgnored()) {
            EXPECT_TRUE(set->getIsKnown()) << set->getShortName().toStdString();
            EXPECT_TRUE(blankController.isKnown(set->getShortName())) << set->getShortName().toStdString();
        }
    }

    // The follow-up reload re-parses with the sets enabled, restoring printings.
    db.reloadCardDatabasesAndNotify();
    ASSERT_FALSE(db.getCardList().value("Cat").isNull());
    EXPECT_TRUE(db.getCardList().value("Cat")->getSets().contains("CAT"));
}

TEST_F(CardDatabaseCacheInvalidationTest, EnableAllUnknownSetsIsBatched)
{
    // The "new sets found" dialog path: enabling the unknown delta must persist
    // with a single flush no matter how many sets the database holds, so users
    // with hundreds of custom sets don't stall the GUI thread. Built directly
    // on CardSet objects because the fixture sets carry no metadata (and would
    // all count as ignored).
    SeededSetPriorityController controller;
    CardSetList sets;
    sets.append(
        CardSet::newInstance(&controller, "AAA", "Alpha Set", "core", QDate(2020, 1, 1), CardSet::PriorityPrimary));
    sets.append(
        CardSet::newInstance(&controller, "BBB", "Beta Set", "expansion", QDate(2021, 6, 1), CardSet::PriorityPrimary));
    sets.append(CardSet::newInstance(&controller, "XXX")); // no metadata: ignored

    sets.enableAllUnknownBatched(&controller);

    EXPECT_EQ(controller.individualWrites, 0) << "unknown-set enable must not write per set";
    EXPECT_EQ(controller.batchedWrites, 1) << "unknown-set enable must persist with a single flush";
    for (const CardSetPtr &set : sets) {
        EXPECT_TRUE(set->getEnabled()) << set->getShortName().toStdString();
        EXPECT_TRUE(controller.isEnabled(set->getShortName())) << set->getShortName().toStdString();
        if (set->getIsKnownIgnored()) {
            EXPECT_FALSE(set->getIsKnown()) << set->getShortName().toStdString();
        } else {
            EXPECT_TRUE(set->getIsKnown()) << set->getShortName().toStdString();
            EXPECT_TRUE(controller.isKnown(set->getShortName())) << set->getShortName().toStdString();
        }
    }
}

TEST_F(CardDatabaseCacheInvalidationTest, MarkAllSetsAsKnownIsBatched)
{
    // The dialog's "No" path: same single-flush requirement as enabling.
    SeededSetPriorityController controller;
    CardSetList sets;
    sets.append(
        CardSet::newInstance(&controller, "AAA", "Alpha Set", "core", QDate(2020, 1, 1), CardSet::PriorityPrimary));
    sets.append(
        CardSet::newInstance(&controller, "BBB", "Beta Set", "expansion", QDate(2021, 6, 1), CardSet::PriorityPrimary));
    sets.append(CardSet::newInstance(&controller, "XXX")); // no metadata: ignored

    sets.markAllAsKnownBatched(&controller);

    EXPECT_EQ(controller.individualWrites, 0) << "mark-known must not write per set";
    EXPECT_EQ(controller.batchedWrites, 1) << "mark-known must persist with a single flush";
    EXPECT_TRUE(controller.isKnown("AAA"));
    EXPECT_TRUE(controller.isKnown("BBB"));
    EXPECT_FALSE(controller.isEnabled("AAA"));
    EXPECT_FALSE(controller.isEnabled("BBB"));
    // Ignored sets keep their known state but are enabled.
    EXPECT_TRUE(controller.isEnabled("XXX"));
}

TEST_F(CardDatabaseCacheInvalidationTest, ReloadReleasesPreviousSnapshot)
{
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    CardInfoPtr card = db.getCardList().value("Cat");
    ASSERT_FALSE(card.isNull());
    QWeakPointer<CardInfo> weakCard(card);
    card.clear();

    db.reloadCardDatabasesAndNotify();

    // CardSet holds strong CardInfoPtrs and CardInfo holds the CardSetPtr through its printings; if that
    // cycle is not broken, the previous snapshot survives the reload.
    EXPECT_TRUE(weakCard.isNull()) << "a reload must release the previous card snapshot";
}

TEST_F(CardDatabaseCacheInvalidationTest, ClearReleasesCards)
{
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    CardInfoPtr card = db.getCardList().value("Cat");
    ASSERT_FALSE(card.isNull());
    QWeakPointer<CardInfo> weakCard(card);
    card.clear();

    db.clear();

    EXPECT_TRUE(weakCard.isNull()) << "clear() must release cards held by set membership";
}

TEST_F(CardDatabaseCacheInvalidationTest, RemoveCardDetachesItFromItsSet)
{
    // removeCard() is the one teardown path a caller can trigger for a single card (the token editor
    // uses it). Without the detach the set's strong reference keeps the removed card alive forever.
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    CardInfoPtr card = db.getCardList().value("Cat");
    ASSERT_FALSE(card.isNull());
    const CardSetPtr catSet = db.getSet("CAT");
    ASSERT_TRUE(catSet->contains(card));

    db.removeCard(card);

    EXPECT_FALSE(catSet->contains(card)) << "removeCard() must detach the card from its set";
    QWeakPointer<CardInfo> weakCard(card);
    card.clear();
    EXPECT_TRUE(weakCard.isNull()) << "a removed card must not stay alive through its set";
}

TEST_F(CardDatabaseCacheInvalidationTest, RemoveSelfPointerStillNotifiesDatabaseCard)
{
    // cardInfoChanged() used to be emitted through a self pointer stored on the card. Without it the
    // card must still notify its observers whenever its properties change.
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    const CardInfoPtr card = db.getCardList().value("Cat");
    ASSERT_FALSE(card.isNull());
    QSignalSpy spy(card.data(), &CardInfo::cardInfoChanged);

    card->setProperty("manacost", "1G");

    ASSERT_EQ(spy.count(), 1) << "a property change on a database card must still emit cardInfoChanged";
    const QList<QVariant> arguments = spy.takeFirst();
    ASSERT_EQ(arguments.size(), 1);
    EXPECT_EQ(arguments.first().value<CardInfoPtr>().data(), card.data())
        << "the signal must carry the card that changed";
}

TEST_F(CardDatabaseCacheInvalidationTest, RemoveSelfPointerCloneNotifiesAsItself)
{
    // A clone owns its own shared reference, so its changes must be reported as the clone's, never the
    // original's: decks and views listening to the original must not be invalidated by a clone edit.
    NoopCardPreferenceProvider prefs;
    SandboxPathProvider pathProvider(root);
    CardDatabase db(nullptr, &prefs, &pathProvider, &controller);

    db.loadCardDatabases();
    const CardInfoPtr original = db.getCardList().value("Cat");
    ASSERT_FALSE(original.isNull());
    const CardInfoPtr clone = original->clone();
    QSignalSpy originalSpy(original.data(), &CardInfo::cardInfoChanged);
    QSignalSpy cloneSpy(clone.data(), &CardInfo::cardInfoChanged);

    clone->setProperty("manacost", "1G");

    ASSERT_EQ(cloneSpy.count(), 1) << "the clone must notify its own observers";
    EXPECT_EQ(cloneSpy.first().first().value<CardInfoPtr>().data(), clone.data())
        << "the clone's signal must carry the clone";
    EXPECT_EQ(originalSpy.count(), 0) << "the clone must not notify the original's observers";
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}