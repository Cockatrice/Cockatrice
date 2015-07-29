#ifndef SERVATRICE_SETTINGS_INTERFACE_H
#define SERVATRICE_SETTINGS_INTERFACE_H

#include <QObject>
#include "server.h"
#include "server_settings_interface.h"

class Servatrice;

class Servatrice_SettingsInterface : public Server_SettingsInterface {
    Q_OBJECT

public:
    Servatrice_SettingsInterface(int _instanceId, Servatrice *_server);
    ~Servatrice_SettingsInterface();
	bool getRequireRegistration();
};

#endif
