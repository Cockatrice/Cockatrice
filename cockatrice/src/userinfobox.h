#ifndef USERINFOBOX_H
#define USERINFOBOX_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class ServerInfo_User;
class AbstractClient;
class Response;

class UserInfoBox : public QWidget
{
    Q_OBJECT
private:
    AbstractClient *client;
    bool editable;
    QLabel avatarLabel, nameLabel, realNameLabel1, realNameLabel2, countryLabel1, countryLabel2, countryLabel3,
        userLevelLabel1, userLevelLabel2, userLevelLabel3, accountAgeLabel1, accountAgeLabel2;
    QPushButton editButton, passwordButton, avatarButton;
    QPixmap avatarPixmap;

public:
    UserInfoBox(AbstractClient *_client, bool editable, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    void retranslateUi();
private slots:
    void processResponse(const Response &r);
    void processEditResponse(const Response &r);
    void processPasswordResponse(const Response &r);
    void processAvatarResponse(const Response &r);

    void actEdit();
    void actEditInternal(const Response &r);
    void actPassword();
    void actAvatar();
public slots:
    void updateInfo(const ServerInfo_User &user);
    void updateInfo(const QString &userName);

private:
    void resizeEvent(QResizeEvent *event) override;
};

#endif
