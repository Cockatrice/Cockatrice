#include "client/settings/cache_settings.h"
#include "interface/card_picture_loader/card_picture_loader_local.h"

#include "gtest/gtest.h"
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <libcockatrice/card/lazy_properties_hash.h>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/card/set/card_set.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <libcockatrice/settings/paths_settings.h>

namespace
{

/**
 * @brief Builds an ExactCard with the requested identity fields.
 *
 * Mirrors how the client constructs cards: the set short name feeds tryLoad()'s
 * setName, and the "num" printing property feeds the collector number.
 */
ExactCard cardFor(const QString &name, const QString &setShortName, const QString &collectorNumber)
{
    CardSetPtr set;
    if (!setShortName.isEmpty()) {
        set = CardSet::newInstance(new NoopCardSetPriorityController(), setShortName, setShortName);
    }

    LazyPropertiesHash properties;
    if (!collectorNumber.isEmpty()) {
        properties.insert("num", collectorNumber);
    }

    return ExactCard(CardInfo::newInstance(name), PrintingInfo(set, properties));
}

class LocalMatcherTest : public ::testing::Test
{
protected:
    QTemporaryDir tempDir;                    ///< Sandboxed "pics" root for every test.
    CardPictureLoaderLocal *loader = nullptr; ///< Constructed per test against the sandboxed paths.

    QString picsPath() const
    {
        return tempDir.path() + "/pics";
    }

    void SetUp() override
    {
        // The loader ctor snapshots the global picture paths once, so point them at the
        // sandbox before constructing it.
        SettingsCache::instance().paths().setPicsPath(picsPath());
        SettingsCache::instance().paths().setCustomPicsPath(picsPath() + "/CUSTOM/");

        loader = new CardPictureLoaderLocal(nullptr);
    }

    void TearDown() override
    {
        delete loader;
        loader = nullptr;
    }

    /**
     * @brief Writes a valid 1x1 PNG under the sandboxed pics path.
     */
    void writePngUnderPics(const QString &relativePath, const QColor &color = Qt::red)
    {
        const QString fullPath = picsPath() + "/" + relativePath;
        ASSERT_TRUE(QDir().mkpath(QFileInfo(fullPath).absolutePath()));

        QImage image(1, 1, QImage::Format_RGB32);
        image.fill(color);

        QImageWriter writer(fullPath, "PNG");
        ASSERT_TRUE(writer.write(image));
    }
};

TEST_F(LocalMatcherTest, ExactMatchBareFileWinsOverSuffixedCompanion)
{
    writePngUnderPics("downloadedPics/TestCard.png");
    writePngUnderPics("downloadedPics/TestCard (1).png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "", ""));

    EXPECT_FALSE(image.isNull()) << "The bare TestCard.png must be picked over its suffixed companion";
}

TEST_F(LocalMatcherTest, SuffixedFileWithoutExactMatchIsNotLoaded)
{
    // The pre-refactor prefix match would have accepted "TestCard (1).png" for "TestCard".
    writePngUnderPics("downloadedPics/TestCard (1).png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "", ""));

    EXPECT_TRUE(image.isNull()) << "A suffixed file must not satisfy an exact card-name lookup";
}

TEST_F(LocalMatcherTest, SetFolderLookupIgnoresSuffixedFiles)
{
    writePngUnderPics("M10/TestCard (1).png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", ""));

    EXPECT_TRUE(image.isNull()) << "Set-folder lookups must also require an exact name match";
}

TEST_F(LocalMatcherTest, SetFolderLookupStillResolvesExactFile)
{
    writePngUnderPics("M10/TestCard.png");
    writePngUnderPics("M10/TestCard (1).png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", ""));

    EXPECT_FALSE(image.isNull()) << "The exact file in the set folder must still resolve";
}

TEST_F(LocalMatcherTest, RootDownloadedPicsFallbackResolvesSchemeFilename)
{
    // Non-set-folder export schemes (Name_Set_Collector) write straight into downloadedPics/.
    writePngUnderPics("downloadedPics/TestCard_M10_1.png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", "1"));

    EXPECT_FALSE(image.isNull()) << "downloadedPics/TestCard_M10_1.png must resolve via the root fallback";
}

TEST_F(LocalMatcherTest, RootFallbackResolvesDashSeparatedVariant)
{
    writePngUnderPics("downloadedPics/TestCard-M10-1.png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", "1"));

    EXPECT_FALSE(image.isNull()) << "The dash-separated import variant must resolve via the root fallback";
}

TEST_F(LocalMatcherTest, DownloadedPicsSetSubfolderStillResolves)
{
    writePngUnderPics("downloadedPics/M10/TestCard_M10_1.png");

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", "1"));

    EXPECT_FALSE(image.isNull()) << "The set-subfolder export scheme must keep resolving";
}

TEST_F(LocalMatcherTest, SetFolderCandidateTakesPrecedenceOverRootFallback)
{
    writePngUnderPics("M10/TestCard_M10_1.png", Qt::red);
    writePngUnderPics("downloadedPics/TestCard_M10_1.png", Qt::blue);

    const QImage image = loader->tryLoad(cardFor("TestCard", "M10", "1"));

    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.pixelColor(0, 0), QColor(Qt::red)) << "The set-folder candidate must be preferred";
}

} // namespace

int main(int argc, char **argv)
{
    // Redirect SettingsCache reads/writes (app-data location) away from the real user profile.
    QStandardPaths::setTestModeEnabled(true);

    // Some CI containers run as a uid without a passwd entry (e.g. GitHub's docker
    // runner), so HOME resolves to "/" and the test-mode qttest data dir cannot be
    // created. SettingsCache's QSettings then silently drops every write, reads come
    // back empty, and the paths the loader searches are "". Give the test a writable
    // HOME for the duration of the run so settings behave like on a normal machine.
    QTemporaryDir home;
    if (home.isValid()) {
        qputenv("HOME", home.path().toLocal8Bit());
    }

    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules("card_picture_loader.*=false\nsettings_cache.*=false");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}