/**
 * @file epoch_format.h
 * @brief Shared timestamp formatting for the moderation tables.
 */

#ifndef EPOCH_FORMAT_H
#define EPOCH_FORMAT_H

#include <QDateTime>
#include <QString>

/**
 * @brief Format a unix timestamp the way the moderation tabs show dates.
 *
 * @param ts          Seconds since the epoch; 0 means "unknown".
 * @param unknownText Label returned for 0, translated by the caller so the
 *                    translation context stays with the tab.
 */
inline QString formatEpochTimestamp(quint64 ts, const QString &unknownText)
{
    if (ts == 0) {
        return unknownText;
    }
    return QDateTime::fromSecsSinceEpoch(ts).toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
}

#endif // EPOCH_FORMAT_H
