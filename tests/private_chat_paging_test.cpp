#include "private_chat_paging.h"

#include <gtest/gtest.h>

namespace
{
ServerInfo_PrivateChatMessage messageAt(quint64 time, const QString &text)
{
    ServerInfo_PrivateChatMessage message;
    message.set_time(time);
    message.set_sender_name("sender");
    message.set_message(text.toStdString());
    return message;
}

QList<ServerInfo_PrivateChatMessage> newestFirst(std::initializer_list<quint64> times)
{
    QList<ServerInfo_PrivateChatMessage> messages;
    for (const quint64 time : times) {
        messages.append(messageAt(time, QString::number(time)));
    }
    return messages;
}

QList<quint64> timesOf(const QList<ServerInfo_PrivateChatMessage> &messages)
{
    QList<quint64> times;
    for (const auto &message : messages) {
        times.append(message.time());
    }
    return times;
}
} // namespace

TEST(PrivateChatPaging, EmptyInputProducesEmptyPage)
{
    const PrivateChatPage page = assemblePrivateChatPage({}, 50);

    EXPECT_TRUE(page.messages.isEmpty());
    EXPECT_FALSE(page.hasMore);
    EXPECT_FALSE(page.needsWholeSecond);
}

TEST(PrivateChatPaging, PageShorterThanLimitIsReturnedOldestFirst)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({300, 200, 100}), 50);

    EXPECT_EQ(timesOf(page.messages), QList<quint64>({100, 200, 300}));
    EXPECT_FALSE(page.hasMore);
    EXPECT_FALSE(page.needsWholeSecond);
}

TEST(PrivateChatPaging, PageExactlyAtLimitIsReturnedWithoutMore)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({300, 200, 100}), 3);

    EXPECT_EQ(timesOf(page.messages), QList<quint64>({100, 200, 300}));
    EXPECT_FALSE(page.hasMore);
}

TEST(PrivateChatPaging, DistinctSecondsKeepWholePage)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({300, 200, 100}), 2);

    EXPECT_EQ(timesOf(page.messages), QList<quint64>({200, 300}));
    EXPECT_TRUE(page.hasMore);
    EXPECT_FALSE(page.needsWholeSecond);
}

TEST(PrivateChatPaging, BoundarySecondIsDeferredToNextPage)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({300, 200, 200}), 2);

    EXPECT_EQ(timesOf(page.messages), QList<quint64>({300}));
    EXPECT_TRUE(page.hasMore);
    EXPECT_FALSE(page.needsWholeSecond);
}

TEST(PrivateChatPaging, WholeSecondIsRequestedWhenNothingElseFits)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({200, 200, 200}), 2);

    EXPECT_TRUE(page.messages.isEmpty());
    EXPECT_TRUE(page.hasMore);
    EXPECT_TRUE(page.needsWholeSecond);
    EXPECT_EQ(page.boundarySecond, 200);
}

TEST(PrivateChatPaging, WholeSecondIsRequestedForSingleMessageLimit)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({200, 200}), 1);

    EXPECT_TRUE(page.messages.isEmpty());
    EXPECT_TRUE(page.needsWholeSecond);
    EXPECT_EQ(page.boundarySecond, 200);
}

TEST(PrivateChatPaging, BoundarySecondAtPageEdgeIsDropped)
{
    const PrivateChatPage page = assemblePrivateChatPage(newestFirst({400, 400, 300, 300}), 3);

    EXPECT_EQ(timesOf(page.messages), QList<quint64>({400, 400}));
    EXPECT_TRUE(page.hasMore);
    EXPECT_FALSE(page.needsWholeSecond);
}

TEST(PrivateChatPaging, ReturnedMessagesKeepSenderAndText)
{
    const PrivateChatPage page = assemblePrivateChatPage({messageAt(200, "hello"), messageAt(100, "world")}, 50);

    ASSERT_EQ(page.messages.size(), 2);
    EXPECT_EQ(page.messages.at(0).message(), "world");
    EXPECT_EQ(page.messages.at(1).message(), "hello");
    EXPECT_EQ(page.messages.at(0).sender_name(), "sender");
}
