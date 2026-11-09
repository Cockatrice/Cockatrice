#ifndef PRIVATE_CHAT_PAGING_H
#define PRIVATE_CHAT_PAGING_H

#include <QList>
#include <libcockatrice/protocol/pb/serverinfo_private_chat_message.pb.h>

/** One page of private chat history, ready for the response. */
struct PrivateChatPage
{
    /** Messages for the response, oldest first. */
    QList<ServerInfo_PrivateChatMessage> messages;
    /** Whether messages older than this page may still exist. */
    bool hasMore = false;
    /**
     * Set when every fetched message belongs to the same second, so the caller
     * has to fetch that whole second before it can answer. messages is empty in
     * that case and boundarySecond names the second to fetch.
     */
    bool needsWholeSecond = false;
    quint64 boundarySecond = 0;
};

/**
 * Assembles one newest-first page from up to limit + 1 messages ordered newest
 * first. A second is never split across pages: when the limit would cut into a
 * second, that whole second is deferred to the next page instead.
 */
PrivateChatPage assemblePrivateChatPage(const QList<ServerInfo_PrivateChatMessage> &newestFirst, int limit);

#endif // PRIVATE_CHAT_PAGING_H
