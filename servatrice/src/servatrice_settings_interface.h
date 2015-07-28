#ifndef SERVATRICE_DATABASE_INTERFACE_H
#define SERVATRICE_DATABASE_INTERFACE_H

#include <QObject>
#include "server.h"
#include "server_settings_interface.h"

class Servatrice;

class Servatrice_SettingsInterface : public Server_SettingsInterface {
    Q_OBJECT

public:
	bool getRequireRegistration();
};

#endif
