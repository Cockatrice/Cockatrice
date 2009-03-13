#ifndef SERVERRESPONSE_H
#define SERVERRESPONSE_H

enum ServerErrorMessage {
	msgNone,
	msgSyntaxError
};

class ServerResponse {
private:
	int msgId;
	bool ok;
	ServerErrorMessage message;
public:
	ServerResponse(int _msgId, bool _ok, ServerErrorMessage _message)
		: msgId(_msgId), ok(_ok), message(_message) { }
	int getMsgId() { return msgId; }
	bool getOk() { return ok; }
	ServerErrorMessage getMessage() { return message; }
};

#endif
