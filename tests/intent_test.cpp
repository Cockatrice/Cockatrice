#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QPointer>
#include <libcockatrice/utility/intent.h>

// StubIntent and PendingIntent live at file scope (not in an anonymous
// namespace) so moc handles them straightforwardly across all supported Qt
// versions.

class StubIntent : public Intent
{
    Q_OBJECT
public:
    explicit StubIntent(QObject *parent = nullptr) : Intent(parent)
    {
    }
    bool executed{false};

protected:
    void doExecute() override
    {
        executed = true;
        emitFinished(true);
    }
};

class PendingIntent : public Intent
{
    Q_OBJECT
public:
    explicit PendingIntent(QObject *parent = nullptr) : Intent(parent)
    {
    }

protected:
    void doExecute() override
    {
        // intentionally never emits finished()
    }
};

// Emits finished(true) then finished(false) back-to-back to exercise the
// finish-once guard.
class DoubleEmitIntent : public Intent
{
    Q_OBJECT
public:
    explicit DoubleEmitIntent(QObject *parent = nullptr) : Intent(parent)
    {
    }

protected:
    void doExecute() override
    {
        emitFinished(true);
        emitFinished(false); // must be a no-op
    }
};

TEST(IntentTest, SelfDeletesAfterFinished)
{
    QPointer<StubIntent> weak = new StubIntent;
    ASSERT_FALSE(weak.isNull());

    weak->execute();
    ASSERT_TRUE(weak->executed) << "doExecute() must be called synchronously by execute()";

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    ASSERT_TRUE(weak.isNull()) << "Intent must delete itself after finished() fires";
}

TEST(IntentTest, DoesNotDeleteBeforeFinished)
{
    QPointer<PendingIntent> weak = new PendingIntent;
    weak->execute();

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    ASSERT_FALSE(weak.isNull()) << "Intent must stay alive while in-flight";

    // Clean up manually for test hygiene.
    delete weak.data();
}

TEST(IntentTest, AbortDeletesIntent)
{
    // abort() emits finished(false) without execute() being called.  The
    // self-delete connection is wired in the constructor, so the intent
    // should clean itself up regardless.
    QPointer<PendingIntent> weak = new PendingIntent;
    ASSERT_FALSE(weak.isNull());

    weak->abort();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    ASSERT_TRUE(weak.isNull()) << "Aborted intent must self-delete";
}

TEST(IntentTest, AbortChainPropagates)
{
    // Build a tiny two-stage chain: head fails, mid should abort and be
    // deleted along with head.  Mirrors the failure-propagation pattern in
    // UrlParser without depending on cockatrice GUI types.
    QPointer<PendingIntent> head = new PendingIntent;
    QPointer<PendingIntent> mid = new PendingIntent;

    QObject::connect(head.data(), &Intent::finished, mid.data(), [m = mid.data()](bool ok) {
        if (ok)
            m->execute();
        else
            m->abort();
    });

    head->abort();

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    ASSERT_TRUE(head.isNull()) << "Head intent must self-delete after abort";
    ASSERT_TRUE(mid.isNull()) << "Mid intent must self-delete after chained abort";
}

TEST(IntentTest, DeletedByParentBeforeFinished)
{
    // Simulates the "user closes Cockatrice mid-flow" path: an intent that
    // never reaches finished() must die cleanly when its QObject parent
    // (typically MainWindow) is destroyed, with no signal emission, no
    // crash, and no leaked timer.
    auto *parent = new QObject;
    QPointer<PendingIntent> weak = new PendingIntent(parent);
    weak->execute(); // never emits finished
    ASSERT_FALSE(weak.isNull());

    delete parent; // simulates MainWindow destruction
    ASSERT_TRUE(weak.isNull()) << "Intent must die with its parent, even mid-flight";
}

TEST(IntentTest, FinishedEmitsAtMostOnce)
{
    // Regression: before the m_finished gate, a concrete intent that emitted
    // finished() from multiple paths (success signal, disconnect, timeout)
    // could deliver finished() twice to chain listeners.
    auto *intent = new DoubleEmitIntent;
    int finishedCount = 0;
    bool firstValue = false;
    QObject::connect(intent, &Intent::finished, [&](bool ok) {
        if (finishedCount == 0)
            firstValue = ok;
        ++finishedCount;
    });

    intent->execute();

    ASSERT_EQ(finishedCount, 1) << "finished() must be emitted exactly once even on duplicate emitFinished calls";
    ASSERT_TRUE(firstValue) << "First emission wins (true)";

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST(IntentTest, ExecuteIsIdempotent)
{
    // Regression: calling execute() twice must not re-enter doExecute().
    class CountingIntent : public Intent
    {
    public:
        int calls{0};

    protected:
        void doExecute() override
        {
            ++calls;
        }
    };

    auto *intent = new CountingIntent;
    intent->execute();
    intent->execute();
    intent->execute();
    ASSERT_EQ(intent->calls, 1) << "execute() must be a no-op after the first call";
    delete intent;
}

#include "intent_test.moc"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
