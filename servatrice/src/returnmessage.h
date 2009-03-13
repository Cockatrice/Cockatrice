#ifndef RETURNMESSAGE_H
#define RETURNMESSAGE_H

#include <QString>
#include <QObject>

class ReturnMessage : public QObject {
	Q_OBJECT
private:
	unsigned int msg_id;
	QString cmd;
public:
	ReturnMessage(QObject *parent = 0) : QObject(parent), msg_id(0) { }
	unsigned int getMsgId() const { return msg_id; }
	void setMsgId(unsigned int _msg_id);
	void setCmd(const QString &_cmd) { cmd = _cmd; }
	bool send(const QString &args = QString(), bool success = true);
	bool sendList(const QStringList &args);
};

#endif
