#ifndef CARD_UPDATE_PROGRESS_PARSER_H
#define CARD_UPDATE_PROGRESS_PARSER_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

/**
 * @brief One progress line read from the card database updater's stdout.
 */
struct CardUpdateProgress
{
    QString stage; ///< one of "download", "scan" or "import"
    qint64 done = 0;
    qint64 total = 0;
};

/**
 * @brief Parses one line of the updater stdout protocol: "PROGRESS <stage> <done> <total>".
 * @param line a single stdout line, with or without its trailing newline
 * @param out receives the parsed values when the line is valid
 * @return true when @p line is a valid progress line
 */
bool parseCardUpdateProgressLine(const QByteArray &line, CardUpdateProgress &out);

#endif // CARD_UPDATE_PROGRESS_PARSER_H
