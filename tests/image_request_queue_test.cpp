#include "../cockatrice/src/interface/card_picture_loader/card_picture_loader_request_queue.h"

#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QUrl>
#include <gtest/gtest.h>

namespace
{
using TestRequest = QPair<QUrl, QPointer<QObject>>;

QUrl url(int index)
{
    return QUrl(QStringLiteral("https://cards.example/%1").arg(index));
}

TestRequest live(int index, QObject &owner)
{
    return qMakePair(url(index), QPointer<QObject>(&owner));
}

TestRequest cancelled(int index)
{
    return qMakePair(url(index), QPointer<QObject>());
}

QList<QUrl> queuedUrls(const QQueue<TestRequest> &queue)
{
    QList<QUrl> urls;
    for (const auto &entry : queue) {
        urls.append(entry.first);
    }
    return urls;
}
} // namespace

TEST(PictureLoaderRequestQueue, EmptyQueueHasNothingToRetire)
{
    QQueue<TestRequest> queue;
    EXPECT_TRUE(takeCancelledRequests(queue).isEmpty());
    EXPECT_TRUE(queue.isEmpty());
}

TEST(PictureLoaderRequestQueue, LiveRequestsStayQueuedInOrder)
{
    QObject first;
    QObject second;
    QQueue<TestRequest> queue;
    queue.enqueue(live(1, first));
    queue.enqueue(live(2, second));

    EXPECT_TRUE(takeCancelledRequests(queue).isEmpty());
    EXPECT_EQ(queuedUrls(queue), (QList<QUrl>{url(1), url(2)}));
}

TEST(PictureLoaderRequestQueue, CancelledRequestsAreRetiredInQueueOrder)
{
    QObject owner;
    QQueue<TestRequest> queue;
    queue.enqueue(live(1, owner));
    queue.enqueue(cancelled(2));
    queue.enqueue(live(3, owner));
    queue.enqueue(cancelled(4));
    queue.enqueue(cancelled(5));

    // Exactly one status entry is retired per vanished work item, in queue order.
    EXPECT_EQ(takeCancelledRequests(queue), (QList<QUrl>{url(2), url(4), url(5)}));
    // The surviving requests keep their relative order.
    EXPECT_EQ(queuedUrls(queue), (QList<QUrl>{url(1), url(3)}));

    // Nothing is retired twice on a later sweep.
    EXPECT_TRUE(takeCancelledRequests(queue).isEmpty());
    EXPECT_EQ(queuedUrls(queue), (QList<QUrl>{url(1), url(3)}));
}

TEST(PictureLoaderRequestQueue, AllCancelledEmptiesTheQueue)
{
    QQueue<TestRequest> queue;
    queue.enqueue(cancelled(1));
    queue.enqueue(cancelled(2));

    EXPECT_EQ(takeCancelledRequests(queue), (QList<QUrl>{url(1), url(2)}));
    EXPECT_TRUE(queue.isEmpty());
}
