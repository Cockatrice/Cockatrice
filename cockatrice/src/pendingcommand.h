#ifndef PENDINGCOMMAND_H
#define PENDINGCOMMAND_H

#include <QString>

class PendingCommand {
private:
	QString cmd;
	int msgid;
	int time;
public:
	int getMsgId() const { return msgid; }
	QString getCmd() const { return cmd; }
	bool timeout() const { return time > 5; }
	void incTime() { ++time; }
	PendingCommand(const QString &_cmd, int _msgid) : cmd(_cmd), msgid(_msgid), time(0) { }
};
 
#endif
