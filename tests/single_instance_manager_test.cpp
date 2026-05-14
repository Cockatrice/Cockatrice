#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QLocalServer>
#include <QLocalSocket>
#include <QRandomGenerator>
#include <QTimer>
#include <libcockatrice/utility/single_instance_manager.h>

namespace
{

QString uniqueSocketName()
{
    return QStringLiteral("CockatriceTest-") + QString::number(QCoreApplication::applicationPid()) +
           QStringLiteral("-") + QString::number(QRandomGenerator::global()->generate());
}

// Drive resolveStartupRole to completion and return the manager.
// The caller owns the returned manager (parented to @p parent if given).
SingleInstanceManager *makeResolvedPrimary(const QString &socketName, QObject *parent = nullptr)
{
    auto *mgr = new SingleInstanceManager(socketName, parent);
    QEventLoop loop;
    QObject::connect(mgr, &SingleInstanceManager::roleResolved, &loop, &QEventLoop::quit);
    mgr->resolveStartupRole(QString());
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    return mgr;
}

} // namespace

TEST(SingleInstanceManagerTest, PerUserSocketNameContainsBase)
{
    const QString name = SingleInstanceManager::perUserSocketName(QStringLiteral("CockatriceInstance"));
    ASSERT_TRUE(name.startsWith(QStringLiteral("CockatriceInstance")))
        << "perUserSocketName must preserve the base prefix; got " << qPrintable(name);
}

TEST(SingleInstanceManagerTest, ResolvesAsPrimaryWhenNoneExists)
{
    const QString socketName = uniqueSocketName();
    QLocalServer::removeServer(socketName);

    SingleInstanceManager mgr(socketName);
    bool resolvedForwarded = true;
    QEventLoop loop;
    QObject::connect(&mgr, &SingleInstanceManager::roleResolved, [&](bool forwarded) {
        resolvedForwarded = forwarded;
        loop.quit();
    });
    mgr.resolveStartupRole(QString());

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_FALSE(resolvedForwarded) << "With no existing primary, we must become primary ourselves";
}

TEST(SingleInstanceManagerTest, ForwardsUrlToExistingPrimary)
{
    const QString socketName = uniqueSocketName();
    QLocalServer::removeServer(socketName);

    QObject parent;
    auto *primary = makeResolvedPrimary(socketName, &parent);

    int receivedCount = 0;
    QString receivedUrl;
    QObject::connect(primary, &SingleInstanceManager::urlReceived, [&](const QString &url) {
        ++receivedCount;
        receivedUrl = url;
    });

    SingleInstanceManager secondary(socketName);
    bool secondaryForwarded = false;
    QEventLoop loop;
    // Wait on roleResolved (the terminal event of the handshake) rather than
    // urlReceived: the secondary's roleResolved fires only after the ACK has
    // round-tripped back from the primary, which itself happens after the
    // primary emits urlReceived.  By the time we quit here, all three of
    // secondaryForwarded / receivedCount / receivedUrl are set.
    QObject::connect(&secondary, &SingleInstanceManager::roleResolved, [&](bool forwarded) {
        secondaryForwarded = forwarded;
        loop.quit();
    });

    const QString url = QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748&roomid=1&gameid=42");
    secondary.resolveStartupRole(url);

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    ASSERT_TRUE(secondaryForwarded) << "Secondary should resolve as forwarded when primary exists";
    ASSERT_EQ(receivedCount, 1) << "urlReceived should fire exactly once on the primary";
    ASSERT_EQ(receivedUrl, url);
}

TEST(SingleInstanceManagerTest, RoleResolvedEmitsAtMostOnce)
{
    // Regression: the probe-side shared flag must keep roleResolved single-
    // emission even when multiple of QLocalSocket's signals fire (e.g.
    // errorOccurred after a successful readyRead, or a timeout firing in the
    // same tick as the terminal signal).  Pre-fix the flag's storage was
    // delete-then-read on subsequent fires, UB whose only visible symptom on
    // a forgiving allocator was duplicate emission — so we observe that.
    const QString socketName = uniqueSocketName();
    QLocalServer::removeServer(socketName);

    QObject parent;
    auto *primary = makeResolvedPrimary(socketName, &parent);
    Q_UNUSED(primary);

    SingleInstanceManager secondary(socketName);
    int resolvedCount = 0;
    QObject::connect(&secondary, &SingleInstanceManager::roleResolved, [&](bool) { ++resolvedCount; });

    QEventLoop loop;
    QObject::connect(&secondary, &SingleInstanceManager::roleResolved, &loop, &QEventLoop::quit);
    secondary.resolveStartupRole(
        QStringLiteral("cockatrice://joingame?hostname=example.com&port=4748&roomid=1&gameid=42"));

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    // Give any straggling signals (errorOccurred on socket teardown,
    // timeout that may have armed) a chance to fire before we count.
    QCoreApplication::processEvents();

    ASSERT_EQ(resolvedCount, 1) << "roleResolved must fire exactly once across the entire handshake";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
