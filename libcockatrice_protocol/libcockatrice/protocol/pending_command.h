/**
 * @file pending_command.h
 * @ingroup Messages
 */
//! \todo Document this file.

#ifndef PENDING_COMMAND_H
#define PENDING_COMMAND_H

#include <QElapsedTimer>
#include <QVariant>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>

class PendingCommand : public QObject
{
    Q_OBJECT
signals:
    void finished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);

private:
    CommandContainer commandContainer;
    QVariant extraData;
    int ticks;
    QElapsedTimer startTime;

public:
    explicit PendingCommand(const CommandContainer &_commandContainer, QVariant _extraData = QVariant());
    CommandContainer &getCommandContainer();
    void setExtraData(const QVariant &_extraData);
    QVariant getExtraData() const;
    void processResponse(const Response &response);
    int tick();

    /**
     * @brief Starts the round-trip timer. Called by the client thread right
     * before the command container is handed to the transport layer.
     */
    void startTiming();

    /**
     * @return Milliseconds elapsed since startTiming(), or -1 if the timer was
     * never started.
     */
    qint64 elapsedMs() const;
};

#endif
