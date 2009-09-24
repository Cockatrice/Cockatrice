#ifndef RETURNMESSAGE_H
#define RETURNMESSAGE_H

#include <QStringList>

class ReturnMessage : public QObject {
	Q_OBJECT
private:
	unsigned int msg_id;
	QString cmd;
public:
	enum ReturnCode { ReturnNothing, ReturnOk, ReturnNameNotFound, ReturnLoginNeeded, ReturnSyntaxError, ReturnContextError, ReturnPasswordWrong, ReturnSpectatorsNotAllowed };
	ReturnMessage(QObject *parent = 0) : QObject(parent), msg_id(0) { }
	unsigned int getMsgId() const { return msg_id; }
	void setMsgId(unsigned int _msg_id) { msg_id = _msg_id; }
	void setCmd(const QString &_cmd) { cmd = _cmd; }
	bool send(ReturnCode code);
	bool sendList(const QStringList &args, const QString &prefix = QString());
};

#endif
