#include "returnmessage.h"
#include "serversocket.h"

bool ReturnMessage::send(ReturnCode code)
{
	ServerSocket *s = qobject_cast<ServerSocket *>(parent());
	if (!s)
		return false;
	QString returnCodeString;
	switch (code) {
		case ReturnNothing: return true;
		case ReturnOk: returnCodeString = "ok"; break;
		case ReturnNameNotFound: returnCodeString = "name_not_found"; break;
		case ReturnLoginNeeded: returnCodeString = "login_needed"; break;
		case ReturnSyntaxError: returnCodeString = "syntax"; break;
		case ReturnContextError: returnCodeString = "context"; break;
		case ReturnPasswordWrong: returnCodeString = "password"; break;
		case ReturnSpectatorsNotAllowed: returnCodeString = "spectators_not_allowed"; break;
	}
	s->msg(QString("resp|%1|%2").arg(msg_id)
				    .arg(returnCodeString));
	return (code == ReturnOk);
}

bool ReturnMessage::sendList(const QStringList &args, const QString &prefix)
{
	ServerSocket *s = qobject_cast<ServerSocket *>(parent());
	if (!s)
		return false;
	
	QString arg1 = prefix.isEmpty() ? cmd : prefix;
	for (int i = 0; i < args.size(); i++)
		s->msg(QString("%1|%2|%3").arg(arg1)
					  .arg(msg_id)
					  .arg(args[i]));
	return true;
}
