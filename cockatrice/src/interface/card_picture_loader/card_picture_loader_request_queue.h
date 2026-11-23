#ifndef COCKATRICE_CARD_PICTURE_LOADER_REQUEST_QUEUE_H
#define COCKATRICE_CARD_PICTURE_LOADER_REQUEST_QUEUE_H

#include <QList>
#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QUrl>

class CardPictureLoaderWorkerWork;

/**
 * @brief One queued picture request.
 *
 * The work pointer is null once the owning work item was destroyed; the URL still
 * owes its queued status entry a completion, so it must be retired from the queue.
 */
using CardPictureLoaderRequest = QPair<QUrl, QPointer<CardPictureLoaderWorkerWork>>;

/**
 * @brief Removes entry.first for every entry whose second (a QPointer) is null.
 *
 * The removed URLs are returned in queue order and the surviving entries keep
 * theirs. The caller emits one completion per returned URL: every queued request
 * retires exactly one status entry, including the ones whose work item vanished.
 */
template <typename Entry> QList<QUrl> takeCancelledRequests(QQueue<Entry> &queue)
{
    QList<QUrl> cancelled;
    for (int i = 0; i < queue.size();) {
        if (queue.at(i).second) {
            ++i;
        } else {
            cancelled.append(queue.takeAt(i).first);
        }
    }
    return cancelled;
}

#endif // COCKATRICE_CARD_PICTURE_LOADER_REQUEST_QUEUE_H
