/**
 * @file local_client_lifetime_test.cpp
 * @brief Tests that a LocalClient survives the death of the LocalServer it talks to.
 * @ingroup Tests
 */

#include <QCoreApplication>
#include <QDebug>
#include <QMessageLogContext>
#include <QString>
#include <QStringList>
#include <cstdio>
#include <cstring>
#include <gtest/gtest.h>
#include <libcockatrice/network/client/local/local_client.h>
#include <libcockatrice/network/server/local/local_server.h>
#include <libcockatrice/network/server/local/local_server_interface.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <memory>

// The server library refers to this global; the real applications define it in main.cpp.
RNG_Abstract *rng = nullptr;

namespace
{

// GTest's main() creates no QCoreApplication, which QObject::deleteLater() needs.
const QCoreApplication &application()
{
    static int argc = 1;
    static char executable[] = "local_client_lifetime_test";
    static char *argv[] = {executable, nullptr};
    static const QCoreApplication app(argc, argv);
    return app;
}

/// Collects the warnings one logging category emits while this object is alive.
///
/// The handler is process-global and Qt does not nest handlers, so only one capture can be
/// installed at a time; GTest running one case at a time is what makes that safe.
class CategoryWarningCapture
{
public:
    explicit CategoryWarningCapture(const char *_category) : category(_category)
    {
        previousHandler = qInstallMessageHandler(captureMessage);
        s_active = this;
    }

    ~CategoryWarningCapture()
    {
        qInstallMessageHandler(previousHandler);
        s_active = nullptr;
    }

    CategoryWarningCapture(const CategoryWarningCapture &) = delete;
    CategoryWarningCapture &operator=(const CategoryWarningCapture &) = delete;

    const QStringList &warnings() const
    {
        return capturedWarnings;
    }

private:
    static void captureMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        if (s_active != nullptr && type == QtWarningMsg && context.category != nullptr &&
            std::strcmp(s_active->category, context.category) == 0) {
            s_active->capturedWarnings.append(message);
        }

        // Forward everything else, so a failure elsewhere in the test is still visible.
        if (previousHandler != nullptr) {
            previousHandler(type, context, message);
            return;
        }
        std::fprintf(stderr, "%s\n", qPrintable(qFormatLogMessage(type, context, message)));
    }

    const char *category;
    QStringList capturedWarnings;

    static CategoryWarningCapture *s_active;
    static QtMessageHandler previousHandler;
};

CategoryWarningCapture *CategoryWarningCapture::s_active = nullptr;
QtMessageHandler CategoryWarningCapture::previousHandler = nullptr;

class LocalClientLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        (void)application();

        server = std::make_unique<LocalServer>();
        LocalServerInterface *lsi = server->newConnection();
        ASSERT_NE(lsi, nullptr);
        // Building the client logs in and joins room 0 through that interface, exactly as
        // MainWindow::startLocalGame() does.
        client = std::make_unique<LocalClient>(lsi, QStringLiteral("lifetime-test-player"),
                                               QStringLiteral("lifetime-test-client-id"));
    }

    void TearDown() override
    {
        // Either case may already have deleted one of these.
        client.reset();
        server.reset();
    }

    std::unique_ptr<LocalServer> server;
    std::unique_ptr<LocalClient> client;
};

TEST_F(LocalClientLifetimeTest, SendAfterServerDeletedIsDropped)
{
    server.reset(); // also destroys the LocalServerInterface the client was built on

    CategoryWarningCapture capture("local_client");
    // A default container stands in for a command still queued when the local game ended.
    client->sendCommandContainer(CommandContainer());

    ASSERT_EQ(capture.warnings().size(), 1);
    EXPECT_TRUE(capture.warnings().first().contains(QStringLiteral("dropping command")))
        << capture.warnings().first().toStdString();

    // The surviving client is intact: reading its state must not touch the deleted interface.
    EXPECT_EQ(client->getUserName(), QStringLiteral("lifetime-test-player"));
}

TEST_F(LocalClientLifetimeTest, DestroyingClientAfterServerIsSafe)
{
    server.reset();

    CategoryWarningCapture capture("local_client");
    client.reset(); // must neither crash nor touch the destroyed interface

    // The destructor itself must have been silent.
    EXPECT_TRUE(capture.warnings().isEmpty());

    // A working capture proves the emptiness above is real rather than a broken handler.
    qCWarning(LocalClientLog) << "local_client_lifetime_test capture self-check";
    EXPECT_EQ(capture.warnings().size(), 1);
}

} // namespace
