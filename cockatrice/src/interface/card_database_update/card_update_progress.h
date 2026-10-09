#ifndef CARD_UPDATE_PROGRESS_H
#define CARD_UPDATE_PROGRESS_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <optional>

/**
 * @brief One stage of the card database updater's progress protocol.
 */
enum class CardUpdateStage
{
    Download, ///< downloading the source data
    Scan,     ///< parsing the downloaded data
    Import,   ///< importing cards into the database
    Unknown,  ///< a stage the client does not recognize
};

/**
 * @brief One progress line read from the card database updater's stdout.
 */
struct CardUpdateProgress
{
    /**
     * @brief Parses one line of the updater stdout protocol: "PROGRESS <stage> <done> <total>".
     * @param line a single stdout line, with or without its trailing newline
     * @return the parsed progress, or @c std::nullopt when the line is not a valid progress line
     */
    static std::optional<CardUpdateProgress> fromProtocolLine(const QByteArray &line);

    /**
     * @brief Protocol token of the stage ("download", "scan", "import" or "unknown"), for logging.
     */
    QString stageToken() const;

    CardUpdateStage stage = CardUpdateStage::Unknown;
    qint64 done = 0;
    qint64 total = 0;
};

#endif // CARD_UPDATE_PROGRESS_H
