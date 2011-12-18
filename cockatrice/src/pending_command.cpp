#include "pending_command.h"
#include "protocol.h"

void PendingCommand::processResponse(ProtocolResponse *response)
{
	emit finished(response);
	emit finished(response->getResponseCode());
}
