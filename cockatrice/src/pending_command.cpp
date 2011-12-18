#include "pending_command.h"
#include "protocol.h"

void PendingCommand::processResponse(ProtocolResponse *response)
{
	qDebug("processResponse");
	emit finished(response);
	emit finished(response->getResponseCode());
}
