#include "returnmessage.h"
#include "testserversocket.h"

void ReturnMessage::setMsgId(unsigned int _msg_id)
{
	msg_id = _msg_id;
}

bool ReturnMessage::send(const QString &args, bool success)
{
	TestServerSocket *s = qobject_cast<TestServerSocket *>(parent());
	if (!s)
		return false;
	s->msg(QString("resp|%1|%2|%3").arg(msg_id)
				       .arg(success ? "ok" : "err")
				       .arg(args));
	return success;
}

bool ReturnMessage::sendList(const QStringList &args)
{
	TestServerSocket *s = qobject_cast<TestServerSocket *>(parent());
	if (!s)
		return false;
	
	for (int i = 0; i < args.size(); i++)
		s->msg(QString("%1|%2|%3").arg(cmd)
					  .arg(msg_id)
					  .arg(args[i]));
	s->msg(QString("%1|%2|.").arg(cmd).arg(msg_id));
	
	return true;
}
