#include "card_update_progress.h"

#include <QList>

namespace
{
CardUpdateStage parseStage(const QByteArray &token)
{
    if (token == "download") {
        return CardUpdateStage::Download;
    }
    if (token == "scan") {
        return CardUpdateStage::Scan;
    }
    if (token == "import") {
        return CardUpdateStage::Import;
    }
    return CardUpdateStage::Unknown;
}
} // namespace

std::optional<CardUpdateProgress> CardUpdateProgress::fromProtocolLine(const QByteArray &line)
{
    const QByteArray trimmed = line.trimmed();
    if (!trimmed.startsWith("PROGRESS ")) {
        return std::nullopt;
    }
    const QList<QByteArray> parts = trimmed.split(' ');
    if (parts.size() != 4) {
        return std::nullopt;
    }
    bool doneOk = false;
    bool totalOk = false;
    const qint64 done = parts.at(2).toLongLong(&doneOk);
    const qint64 total = parts.at(3).toLongLong(&totalOk);
    if (!doneOk || !totalOk || done < 0 || total < 0) {
        return std::nullopt;
    }
    CardUpdateProgress progress;
    progress.stage = parseStage(parts.at(1));
    progress.done = done;
    progress.total = total;
    return progress;
}

QString CardUpdateProgress::stageToken() const
{
    switch (stage) {
        case CardUpdateStage::Download:
            return QStringLiteral("download");
        case CardUpdateStage::Scan:
            return QStringLiteral("scan");
        case CardUpdateStage::Import:
            return QStringLiteral("import");
        case CardUpdateStage::Unknown:
            return QStringLiteral("unknown");
    }
    return QStringLiteral("unknown");
}
