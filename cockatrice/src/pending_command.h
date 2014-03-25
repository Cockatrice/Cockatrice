#ifndef PENDING_COMMAND_H
#define PENDING_COMMAND_H

#include "pb/commands.pb.h"
#include "pb/response.pb.h"
#include <QVariant>

class PendingCommand : public QObject {
    Q_OBJECT
signals:
    void finished(const Response &response, const CommandContainer &commandContainer, const QVariant &extraData);
    void finished(Response::ResponseCode respCode);
private:
    CommandContainer commandContainer;
    QVariant extraData;
    int ticks;
public:
    PendingCommand(const CommandContainer &_commandContainer, QVariant _extraData = QVariant())
        : commandContainer(_commandContainer), extraData(_extraData), ticks(0) { }
    CommandContainer &getCommandContainer() { return commandContainer; }
    void setExtraData(const QVariant &_extraData) { extraData = _extraData; }
    QVariant getExtraData() const { return extraData; }
    void processResponse(const Response &response)
    {
        emit finished(response, commandContainer, extraData);
        emit finished(response.response_code());
    }
    int tick() { return ++ticks; }
};

#endif 
