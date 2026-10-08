#include "card_update_progress_parser.h"

#include <QList>

bool parseCardUpdateProgressLine(const QByteArray &line, CardUpdateProgress &out)
{
    const QByteArray trimmed = line.trimmed();
    if (!trimmed.startsWith("PROGRESS ")) {
        return false;
    }
    const QList<QByteArray> parts = trimmed.split(' ');
    if (parts.size() != 4) {
        return false;
    }
    bool doneOk = false;
    bool totalOk = false;
    const qint64 done = parts.at(2).toLongLong(&doneOk);
    const qint64 total = parts.at(3).toLongLong(&totalOk);
    if (!doneOk || !totalOk || done < 0 || total < 0) {
        return false;
    }
    out.stage = QString::fromLatin1(parts.at(1));
    out.done = done;
    out.total = total;
    return true;
}
