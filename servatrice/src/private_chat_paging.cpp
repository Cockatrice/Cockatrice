#include "private_chat_paging.h"

#include <algorithm>

PrivateChatPage assemblePrivateChatPage(const QList<ServerInfo_PrivateChatMessage> &newestFirst, int limit)
{
    PrivateChatPage page;
    page.messages = newestFirst;
    page.hasMore = newestFirst.size() > limit;
    if (!page.hasMore) {
        std::reverse(page.messages.begin(), page.messages.end());
        return page;
    }

    // The (limit+1)th message is the first one that does not fit. If its second
    // appears anywhere in the page, that whole second would be split, so drop
    // every message in that second and leave it for the next page.
    const quint64 boundary = newestFirst.at(limit).time();
    while (!page.messages.isEmpty() && page.messages.last().time() == boundary) {
        page.messages.removeLast();
    }

    if (page.messages.isEmpty()) {
        page.needsWholeSecond = true;
        page.boundarySecond = boundary;
        return page;
    }

    std::reverse(page.messages.begin(), page.messages.end());
    return page;
}
