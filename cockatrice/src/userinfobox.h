#ifndef USERINFOBOX_H
#define USERINFOBOX_H

#include <QWidget>

class QLabel;
class ServerInfo_User;
class AbstractClient;
class Response;

class UserInfoBox : public QWidget {
    Q_OBJECT
private:
    AbstractClient *client;
    bool fullInfo;
    QLabel *avatarLabel, *nameLabel, *realNameLabel1, *realNameLabel2, *genderLabel1, *genderLabel2, *countryLabel1, *countryLabel2, *userLevelLabel1, *userLevelLabel2, *userLevelLabel3;
public:
    UserInfoBox(AbstractClient *_client, bool fullInfo, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void retranslateUi();
private slots:
    void processResponse(const Response &r);
public slots:
    void updateInfo(const ServerInfo_User &user);
    void updateInfo(const QString &userName);
};

#endif
