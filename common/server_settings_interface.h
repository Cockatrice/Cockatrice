#ifndef SERVER_SETTINGS_INTERFACE_H
#define SERVER_SETTINGS_INTERFACE_H

#include <QObject>
#include "server.h"

class Server_SettingsInterface : public QObject {
    Q_OBJECT
public:
    Server_SettingsInterface(QObject *parent = 0)
        : QObject(parent) { }

	virtual bool getRequireRegistration() { return false; }
};

#endif
