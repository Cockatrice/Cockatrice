#ifndef PENDING_COMMAND_H
#define PENDING_COMMAND_H

#include "protocol_datastructures.h"
#include "pb/commands.pb.h"
#include <QVariant>

class ProtocolResponse;

class PendingCommand : public QObject {
	Q_OBJECT
signals:
	void finished(ProtocolResponse *response);
	void finished(ResponseCode response);
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
	void processResponse(ProtocolResponse *response);
	int tick() { return ++ticks; }
};

#endif 
