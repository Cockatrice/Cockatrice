#include "client/network/update/client/release_channel.h"
#include "version_string.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <algorithm>
#include <cstring>
#include <functional>
#include <gtest/gtest.h>

namespace
{
const QString RELEASE_HASH(40, QLatin1Char('a'));

class FakeReply : public QNetworkReply
{
public:
    explicit FakeReply(const QNetworkRequest &request, QObject *parent) : QNetworkReply(parent)
    {
        setRequest(request);
        setUrl(request.url());
        open(QIODevice::ReadOnly);
    }

    void complete(const QByteArray &body, int status = 200, NetworkError networkError = NoError)
    {
        content = body;
        offset = 0;
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
        if (networkError != NoError) {
            setError(networkError, QStringLiteral("Request failed"));
        }
        setFinished(true);
        emit finished();
    }

    void abort() override
    {
        if (onAbort) {
            onAbort();
        }
        complete({}, 0, OperationCanceledError);
    }

    qint64 bytesAvailable() const override
    {
        return content.size() - offset + QIODevice::bytesAvailable();
    }

    std::function<void()> onAbort;

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        const qint64 length = std::min(maxSize, content.size() - offset);
        if (length <= 0) {
            return -1;
        }
        std::memcpy(data, content.constData() + offset, static_cast<size_t>(length));
        offset += length;
        return length;
    }

private:
    QByteArray content;
    qint64 offset = 0;
};

class FakeNetworkManager : public QNetworkAccessManager
{
public:
    explicit FakeNetworkManager(QObject *parent) : QNetworkAccessManager(parent)
    {
    }
    QList<QPointer<FakeReply>> requests;

protected:
    QNetworkReply *createRequest(Operation, const QNetworkRequest &request, QIODevice *) override
    {
        auto *reply = new FakeReply(request, this);
        requests.append(reply);
        return reply;
    }
};

template <typename Channel> class TestChannel : public Channel
{
public:
    TestChannel()
    {
        delete this->netMan;
        network = new FakeNetworkManager(this);
        this->netMan = network;
    }
    FakeNetworkManager *network;
};

QJsonObject releaseObject(const QString &tag = QStringLiteral("v1.0"))
{
    return {{"tag_name", tag},
            {"name", "Release " + tag},
            {"html_url", "https://github.com/Cockatrice/Cockatrice/releases/tag/" + tag},
            {"published_at", "2026-10-31T15:10:00Z"},
            {"target_commitish", "master"},
            {"draft", false},
            {"prerelease", false},
            {"assets", QJsonArray{}}};
}

QByteArray commitObject(const QString &hash = RELEASE_HASH)
{
    return QJsonDocument(QJsonObject{{"sha", hash}}).toJson();
}

class ReleaseChannelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        static int argc = 1;
        static char applicationName[] = "release_channel_test";
        static char *argv[] = {applicationName, nullptr};
        static QCoreApplication application(argc, argv);
    }
};

TEST_F(ReleaseChannelTest, ConcurrentStableChecksResolveOneEncodedTagToItsCommit)
{
    TestChannel<StableReleaseChannel> channel;
    int finished = 0;
    bool updateRequired = false;
    QObject::connect(&channel, &ReleaseChannel::finishedCheck, &channel, [&](bool update, bool, Release *) {
        ++finished;
        updateRequired = update;
    });
    channel.checkForUpdates();
    channel.checkForUpdates();
    ASSERT_EQ(channel.network->requests.size(), 1);
    channel.network->requests[0]->complete(QJsonDocument(releaseObject("release/1.0+#")).toJson());
    ASSERT_EQ(channel.network->requests.size(), 2);
    EXPECT_TRUE(channel.network->requests[1]->url().toEncoded().endsWith("/commits/release%2F1.0%2B%23"));
    channel.checkForUpdates();
    EXPECT_EQ(channel.network->requests.size(), 2);
    EXPECT_EQ(channel.getLastRelease(), nullptr);
    channel.network->requests[1]->complete(commitObject());
    ASSERT_EQ(finished, 1);
    ASSERT_NE(channel.getLastRelease(), nullptr);
    EXPECT_EQ(channel.getLastRelease()->getCommitHash(), RELEASE_HASH);
    EXPECT_EQ(channel.getLastRelease()->getPublishDate(), QDate(2026, 10, 31));
    EXPECT_FALSE(channel.getLastRelease()->isCompatibleVersionFound());
    EXPECT_EQ(updateRequired, !RELEASE_HASH.startsWith(QString::fromLatin1(VERSION_COMMIT), Qt::CaseInsensitive));
}

TEST_F(ReleaseChannelTest, BetaUsesResolvedReleaseTagAndSkipsDrafts)
{
    TestChannel<BetaReleaseChannel> channel;
    auto draft = releaseObject("future-draft");
    draft["draft"] = true;
    auto prerelease = releaseObject("beta-build");
    prerelease["prerelease"] = true;
    channel.checkForUpdates();
    channel.network->requests[0]->complete(QJsonDocument(QJsonArray{draft, prerelease}).toJson());
    ASSERT_EQ(channel.network->requests.size(), 2);
    EXPECT_TRUE(channel.network->requests[1]->url().path().endsWith("/commits/beta-build"));
    channel.network->requests[1]->complete(commitObject());
    ASSERT_NE(channel.getLastRelease(), nullptr);
    EXPECT_EQ(channel.getLastRelease()->getCommitHash(), RELEASE_HASH);
    EXPECT_EQ(channel.getLastRelease()->getName(), QStringLiteral("beta-build (aaaaaaa)"));
    EXPECT_TRUE(channel.getLastRelease()->getDescriptionUrl().endsWith("...aaaaaaa"));

    channel.checkForUpdates();
    channel.network->requests[2]->complete(QJsonDocument(QJsonArray{releaseObject("stable-build")}).toJson());
    channel.network->requests[3]->complete(commitObject());
    EXPECT_EQ(channel.getLastRelease()->getName(), QStringLiteral("Release stable-build"));
    EXPECT_EQ(channel.getLastRelease()->getDescriptionUrl(),
              QStringLiteral("https://github.com/Cockatrice/Cockatrice/releases/tag/stable-build"));
}

TEST_F(ReleaseChannelTest, FailedRefreshPreservesCompletedSnapshotAndAllowsRetry)
{
    TestChannel<StableReleaseChannel> channel;
    int errors = 0;
    QObject::connect(&channel, &ReleaseChannel::error, &channel, [&](const QString &) { ++errors; });
    channel.checkForUpdates();
    channel.network->requests[0]->complete(QJsonDocument(releaseObject()).toJson());
    channel.network->requests[1]->complete(commitObject());
    QPointer<Release> previous = channel.getLastRelease();
    ASSERT_TRUE(previous);

    channel.checkForUpdates();
    channel.network->requests[2]->complete(QJsonDocument(releaseObject("v2.0")).toJson());
    channel.network->requests[3]->complete("{\"message\":\"rate limited\"}", 403, QNetworkReply::ContentAccessDenied);
    EXPECT_EQ(errors, 1);
    EXPECT_EQ(channel.getLastRelease(), previous.data());
    EXPECT_EQ(previous->getName(), QStringLiteral("Release v1.0"));

    channel.checkForUpdates();
    channel.network->requests[4]->complete(QJsonDocument(releaseObject("v3.0")).toJson());
    channel.network->requests[5]->complete(commitObject(QString(40, QLatin1Char('b'))));
    EXPECT_NE(channel.getLastRelease(), previous.data());
    EXPECT_EQ(channel.getLastRelease()->getName(), QStringLiteral("Release v3.0"));
    ASSERT_TRUE(previous);
    EXPECT_EQ(previous->getName(), QStringLiteral("Release v1.0"));
}

TEST_F(ReleaseChannelTest, RejectsMalformedHttpAndJsonResponsesWithoutPublishing)
{
    TestChannel<StableReleaseChannel> channel;
    int errors = 0;
    int finished = 0;
    QObject::connect(&channel, &ReleaseChannel::error, &channel, [&](const QString &) { ++errors; });
    QObject::connect(&channel, &ReleaseChannel::finishedCheck, &channel, [&](bool, bool, Release *) { ++finished; });
    const QList<QByteArray> invalidBodies = {"not json", "[]", "{}", "{\"tag_name\":123}"};
    for (const auto &body : invalidBodies) {
        channel.checkForUpdates();
        channel.network->requests.last()->complete(body);
    }
    channel.checkForUpdates();
    channel.network->requests.last()->complete(QJsonDocument(releaseObject()).toJson(), 503);
    channel.checkForUpdates();
    channel.network->requests.last()->complete(QJsonDocument(releaseObject()).toJson());
    channel.network->requests.last()->complete("{\"sha\":\"master\"}");
    EXPECT_EQ(errors, 6);
    EXPECT_EQ(finished, 0);
    EXPECT_EQ(channel.getLastRelease(), nullptr);
}

TEST_F(ReleaseChannelTest, EmptyBetaListIsAnErrorAndCanBeRetried)
{
    TestChannel<BetaReleaseChannel> channel;
    int errors = 0;
    QObject::connect(&channel, &ReleaseChannel::error, &channel, [&](const QString &) { ++errors; });
    channel.checkForUpdates();
    channel.network->requests.last()->complete("[]");
    EXPECT_EQ(errors, 1);
    channel.checkForUpdates();
    EXPECT_EQ(channel.network->requests.size(), 2);
}

TEST_F(ReleaseChannelTest, DestroyingTheChannelAbortsPendingLookupWithoutReportingAnError)
{
    auto channel = std::make_unique<TestChannel<StableReleaseChannel>>();
    bool aborted = false;
    int errors = 0;
    QObject::connect(channel.get(), &ReleaseChannel::error, [&](const QString &) { ++errors; });
    channel->checkForUpdates();
    channel->network->requests.last()->onAbort = [&] { aborted = true; };
    channel.reset();
    EXPECT_TRUE(aborted);
    EXPECT_EQ(errors, 0);
}
} // namespace
