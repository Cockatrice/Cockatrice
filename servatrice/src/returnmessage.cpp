#include "returnmessage.h"
#include "serversocket.h"

bool ReturnMessage::send(ReturnCode code)
{
	ServerSocket *s = qobject_cast<ServerSocket *>(parent());
	if (!s)
		return false;
	bool success = (code == ReturnOk);
	QString returnCodeString;
	switch (code) {
		case ReturnNothing: return true;
		case ReturnOk: break;
		case ReturnLoginNeeded: returnCodeString = "login_needed"; break;
		case ReturnSyntaxError: returnCodeString = "syntax"; break;
		case ReturnContextError: returnCodeString = "context"; break;
		case ReturnPasswordWrong: returnCodeString = "password"; break;
	}
	s->msg(QString("resp|%1|%2|%3").arg(msg_id)
				       .arg(success ? "ok" : "err")
				       .arg(returnCodeString));
	return success;
}

bool ReturnMessage::sendList(const QStringList &args)
{
	ServerSocket *s = qobject_cast<ServerSocket *>(parent());
	if (!s)
		return false;
	
	for (int i = 0; i < args.size(); i++)
		s->msg(QString("%1|%2|%3").arg(cmd)
					  .arg(msg_id)
					  .arg(args[i]));
	s->msg(QString("%1|%2|.").arg(cmd).arg(msg_id));
	
	return true;
}
