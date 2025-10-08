#include "pending_command.h"

PendingCommand::PendingCommand(const CommandContainer &_commandContainer, QVariant _extraData)
    : commandContainer(_commandContainer), extraData(_extraData), ticks(0)
{
}

CommandContainer &PendingCommand::getCommandContainer()
{
    return commandContainer;
}

void PendingCommand::setExtraData(const QVariant &_extraData)
{
    extraData = _extraData;
}

QVariant PendingCommand::getExtraData() const
{
    return extraData;
}

void PendingCommand::processResponse(const Response &response)
{
    emit finished(response, commandContainer, extraData);
}

int PendingCommand::tick()
{
    return ++ticks;
}
