/**
 * @file user_info_box.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef USERINFOBOX_H
#define USERINFOBOX_H

#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/network/server/remote/user_level.h>
#include <libcockatrice/utility/days_years_between.h>

class AbstractClient;
class Response;

class UserInfoBox : public QWidget
{
    Q_OBJECT
private:
    AbstractClient *client;
    bool editable;
    QLabel avatarPic, userLevelIcon, nameLabel, realNameLabel1, realNameLabel2, countryLabel1, countryLabel2,
        countryLabel3, userLevelLabel1, userLevelLabel2, accountAgeLabel1, accountAgeLabel2;
    QPushButton editButton, passwordButton, avatarButton, bannerCardButton;
    QPixmap avatarPixmap;
    bool hasAvatar;
    ServerInfo_User currentUserInfo;
    bool hasUserInfo = false;
    UserLevelFlags userLevel;
    ServerInfo_User::PawnColorsOverride pawnColors;
    QString privLevel;

    static QString getAgeString(int ageSeconds);

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
    void actBannerCard();
public slots:
    void updateInfo(const ServerInfo_User &user);
    void updateInfo(const QString &userName);

private:
    void resizeEvent(QResizeEvent *event) override;
    void changePassword(const QString &oldPassword, const QString &newPassword);
};

#endif
