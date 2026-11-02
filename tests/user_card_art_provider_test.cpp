#include "carddatabase/test_card_database_path_provider.h"
#include "interface/card_picture_loader/card_picture_loader.h"
#include "interface/widgets/server/user/user_card_art_provider.h"

#include <QApplication>
#include <QHash>
#include <QPixmapCache>
#include <QPointer>
#include <QSignalSpy>
#include <QTimer>
#include <gtest/gtest.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

namespace
{
QHash<QString, QPixmap> availablePictures;
QStringList pictureRequests;

QPixmap picture()
{
    QPixmap result(100, 140);
    result.fill(Qt::green);
    return result;
}

ExactCard card(const QString &name)
{
    return CardDatabaseManager::query()->getCard({name, {}});
}

class UserCardArtProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        availablePictures.clear();
        QPixmapCache::clear();
        pictureRequests.clear();
        CardDatabaseManager::getInstance()->clear();
        CardDatabaseManager::getInstance()->loadCardDatabases();
        ASSERT_TRUE(card("Cat"));
        ASSERT_TRUE(card("Dog"));
    }
};

TEST_F(UserCardArtProviderTest, MissingFirstBannerDoesNotBlockDatabaseStartupQueue)
{
    CardDatabaseManager::getInstance()->clear();
    UserCardArtProvider provider;
    provider.requestCardArt("first", "Cat", {});
    provider.requestCardArt("second", "Dog", {});
    EXPECT_TRUE(pictureRequests.isEmpty());
    CardDatabaseManager::getInstance()->loadCardDatabases();
    EXPECT_TRUE(pictureRequests.contains(card("Cat").getPixmapCacheKey()));
    EXPECT_TRUE(pictureRequests.contains(card("Dog").getPixmapCacheKey()));
    availablePictures.insert(card("Dog").getPixmapCacheKey(), picture());
    card("Dog").emitPixmapUpdated();
    EXPECT_TRUE(provider.cache().contains(UserCardArtProvider::cacheKey("second", "Dog", {})));
}

TEST_F(UserCardArtProviderTest, CachedLoaderFailureReleasesImmediatelyForLaterRetry)
{
    UserCardArtProvider provider;
    QPixmapCache::insert(card("Cat").getPixmapCacheKey(), QPixmap{});
    provider.requestCardArt("user", "Cat", {});
    const int before = pictureRequests.size();
    provider.requestCardArt("user", "Cat", {});
    EXPECT_EQ(pictureRequests.size(), before + 1);
    QPixmapCache::remove(card("Cat").getPixmapCacheKey());
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    provider.requestCardArt("user", "Cat", {});
    EXPECT_EQ(provider.cache().size(), 1);
}

TEST_F(UserCardArtProviderTest, OtherPrintingCannotCompleteCurrentBanner)
{
    UserCardArtProvider provider;
    QSignalSpy updated(&provider, &UserCardArtProvider::cardArtUpdated);
    provider.requestCardArt("user", "Cat", {});
    PrintingInfo unrelated;
    unrelated.setProperty("uuid", "another-printing");
    card("Cat").getCardPtr()->pixmapUpdated(unrelated);
    EXPECT_EQ(updated.count(), 0);
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    card("Cat").emitPixmapUpdated();
    EXPECT_EQ(updated.count(), 1);
    EXPECT_EQ(provider.cache().size(), 1);
}

TEST_F(UserCardArtProviderTest, DeadlineAllowsRetryWhenLoaderSuppressesCompletion)
{
    UserCardArtProvider provider;
    provider.requestCardArt("user", "Cat", {});
    auto timers = provider.findChildren<QTimer *>();
    ASSERT_EQ(timers.size(), 1);
    ASSERT_TRUE(QMetaObject::invokeMethod(timers.first(), "timeout", Qt::DirectConnection));
    const int before = pictureRequests.size();
    provider.requestCardArt("user", "Cat", {});
    EXPECT_EQ(pictureRequests.size(), before + 1);
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    card("Cat").emitPixmapUpdated();
    EXPECT_EQ(provider.cache().size(), 1);
}

TEST_F(UserCardArtProviderTest, ReplacedAndRemovedBannerRejectLateCompletion)
{
    UserCardArtProvider provider;
    provider.requestCardArt("user", "Cat", {});
    provider.requestCardArt("user", "Dog", {});
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    card("Cat").emitPixmapUpdated();
    EXPECT_TRUE(provider.cache().isEmpty());
    availablePictures.insert(card("Dog").getPixmapCacheKey(), picture());
    card("Dog").emitPixmapUpdated();
    EXPECT_EQ(provider.cache().size(), 1);
    provider.requestCardArt("user", {}, {});
    EXPECT_TRUE(provider.cache().isEmpty());
    card("Dog").emitPixmapUpdated();
    EXPECT_TRUE(provider.cache().isEmpty());
}

TEST_F(UserCardArtProviderTest, DatabaseReplacementRebindsPendingAndCachedBanners)
{
    UserCardArtProvider provider;
    const ExactCard oldCat = card("Cat");
    availablePictures.insert(card("Dog").getPixmapCacheKey(), picture());
    provider.requestCardArt("pending", "Cat", {});
    provider.requestCardArt("cached", "Dog", {});
    ASSERT_EQ(provider.cache().size(), 1);
    availablePictures.clear();
    CardDatabaseManager::getInstance()->clear();
    CardDatabaseManager::getInstance()->loadCardDatabases();
    EXPECT_NE(card("Cat").getCardPtr(), oldCat.getCardPtr());
    EXPECT_TRUE(provider.cache().isEmpty());
    availablePictures.insert(oldCat.getPixmapCacheKey(), picture());
    oldCat.emitPixmapUpdated();
    EXPECT_TRUE(provider.cache().isEmpty());
    card("Cat").emitPixmapUpdated();
    EXPECT_EQ(provider.cache().size(), 1);
    availablePictures.insert(card("Dog").getPixmapCacheKey(), picture());
    card("Dog").emitPixmapUpdated();
    EXPECT_EQ(provider.cache().size(), 2);
}

TEST_F(UserCardArtProviderTest, CacheIdentityDoesNotParseUserProvidedSeparators)
{
    EXPECT_NE(UserCardArtProvider::cacheKey("a|b", "c", "d"), UserCardArtProvider::cacheKey("a", "b|c", "d"));
    UserCardArtProvider provider;
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    provider.requestCardArt("user|name", "Cat", {});
    EXPECT_EQ(provider.cache().size(), 1);
    EXPECT_TRUE(provider.cache().contains(UserCardArtProvider::cacheKey("user|name", "Cat", {})));
}

TEST_F(UserCardArtProviderTest, CompletionMayDestroyProvider)
{
    availablePictures.insert(card("Cat").getPixmapCacheKey(), picture());
    auto *provider = new UserCardArtProvider;
    QPointer<UserCardArtProvider> guard(provider);
    QObject::connect(provider, &UserCardArtProvider::cardArtUpdated, provider, [provider] { delete provider; });
    provider->requestCardArt("user", "Cat", {});
    EXPECT_TRUE(guard.isNull());
}
} // namespace

// Exercise the actual provider, CardDatabase, exact-printing signals and Qt
// lifetimes without starting an image worker or reaching an external service.
void CardPictureLoader::getPixmap(QPixmap &pixmap, const ExactCard &exact, QSize)
{
    pictureRequests.append(exact.getPixmapCacheKey());
    pixmap = availablePictures.value(exact.getPixmapCacheKey());
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    static TestCardDatabasePathProvider paths;
    CardDatabaseManager::setCardDatabasePathProvider(&paths);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
