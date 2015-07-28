#ifndef SERVER_SETTINGS_INTERFACE_H
#define SERVER_SETTINGS_INTERFACE_H

#include <QObject>
#include "server.h"

class Server_SettingsInterface : public QObject {
    Q_OBJECT
public:
	virtual bool getRequireRegistration() { return false; }
};

#endif
