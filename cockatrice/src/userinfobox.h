#ifndef USERINFOBOX_H
#define USERINFOBOX_H

#include <QWidget>

class QLabel;
class ServerInfo_User;
class AbstractClient;
class ProtocolResponse;

class UserInfoBox : public QWidget {
	Q_OBJECT
private:
	AbstractClient *client;
	bool fullInfo;
	QLabel *avatarLabel, *nameLabel, *realNameLabel1, *realNameLabel2, *countryLabel1, *countryLabel2, *userLevelLabel1, *userLevelLabel2, *userLevelLabel3;
public:
	UserInfoBox(AbstractClient *_client, bool fullInfo, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	void retranslateUi();
private slots:
	void processResponse(ProtocolResponse *r);
public slots:
	void updateInfo(ServerInfo_User *user);
	void updateInfo(const QString &userName);
};

#endif