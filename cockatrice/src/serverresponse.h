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
	int getMsgId() const { return msgId; }
	bool getOk() const { return ok; }
	ServerErrorMessage getMessage() const { return message; }
};

#endif
