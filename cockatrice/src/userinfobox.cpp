#include "userinfobox.h"
#include "pixmapgenerator.h"
#include "abstractclient.h"
#include "dlg_edit_user.h"
#include "dlg_edit_password.h"
#include "dlg_edit_avatar.h"

#include <QLabel>
#include <QDateTime>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>

#include "pending_command.h"
#include "pb/session_commands.pb.h"
#include "pb/response_get_user_info.pb.h"

const qint64 SIXTY = 60;
const qint64 HOURS_IN_A_DAY = 24;
const qint64 DAYS_IN_A_YEAR = 365;

UserInfoBox::UserInfoBox(AbstractClient *_client, bool _editable, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags), client(_client), editable(_editable)
{
    QFont nameFont = nameLabel.font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() * 1.5);
    nameLabel.setFont(nameFont);

    avatarLabel.setMaximumWidth(400);
    avatarLabel.setMaximumHeight(200);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(&avatarLabel, 0, 0, 1, 3, Qt::AlignCenter);
    mainLayout->addWidget(&nameLabel, 1, 0, 1, 3);
    mainLayout->addWidget(&realNameLabel1, 2, 0, 1, 1);
    mainLayout->addWidget(&realNameLabel2, 2, 1, 1, 2);
    mainLayout->addWidget(&countryLabel1, 4, 0, 1, 1);
    mainLayout->addWidget(&countryLabel2, 4, 1, 1, 2);
    mainLayout->addWidget(&countryLabel3, 4, 2, 1, 1);
    mainLayout->addWidget(&userLevelLabel1, 5, 0, 1, 1);
    mainLayout->addWidget(&userLevelLabel2, 5, 1, 1, 1);
    mainLayout->addWidget(&userLevelLabel3, 5, 2, 1, 1);
    mainLayout->addWidget(&accountAgeLebel1, 6, 0, 1, 1);
    mainLayout->addWidget(&accountAgeLabel2, 6, 2, 1, 1);
    mainLayout->setColumnStretch(2, 10);

    if(editable)
    {
        QHBoxLayout * buttonsLayout = new QHBoxLayout;
        buttonsLayout->addWidget(&editButton);
        buttonsLayout->addWidget(&passwordButton);
        buttonsLayout->addWidget(&avatarButton);
        mainLayout->addLayout(buttonsLayout, 7, 0, 1, 3);

        connect(&editButton, SIGNAL(clicked()), this, SLOT(actEdit()));
        connect(&passwordButton, SIGNAL(clicked()), this, SLOT(actPassword()));
        connect(&avatarButton, SIGNAL(clicked()), this, SLOT(actAvatar()));
    }

    setWindowTitle(tr("User information"));
    setLayout(mainLayout);
    retranslateUi();
}

void UserInfoBox::retranslateUi()
{
    realNameLabel1.setText(tr("Real name:"));
    countryLabel1.setText(tr("Location:"));
    userLevelLabel1.setText(tr("User level:"));
    accountAgeLebel1.setText(tr("Account Age:"));

    editButton.setText(tr("Edit"));
    passwordButton.setText(tr("Change password"));
    avatarButton.setText(tr("Change avatar"));
}

void UserInfoBox::updateInfo(const ServerInfo_User &user)
{
    const UserLevelFlags userLevel(user.user_level());

    QPixmap avatarPixmap;
    const std::string bmp = user.avatar_bmp();
    if (!avatarPixmap.loadFromData((const uchar *) bmp.data(), bmp.size()))
        avatarPixmap = UserLevelPixmapGenerator::generatePixmap(64, userLevel, false, QString::fromStdString(user.privlevel()));
    avatarLabel.setPixmap(avatarPixmap.scaled(avatarLabel.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    nameLabel.setText(QString::fromStdString(user.name()));
    realNameLabel2.setText(QString::fromStdString(user.real_name()));
    QString country = QString::fromStdString(user.country());

    if (country.length() != 0)
    {
        countryLabel2.setPixmap(CountryPixmapGenerator::generatePixmap(15, country));
        countryLabel3.setText(QString("(%1)").arg(country.toUpper()));
    }
    else
    {
        countryLabel2.setText("");
        countryLabel3.setText("");
    }

    userLevelLabel2.setPixmap(UserLevelPixmapGenerator::generatePixmap(15, userLevel, false, QString::fromStdString(user.privlevel())));
    QString userLevelText;
    if (userLevel.testFlag(ServerInfo_User::IsAdmin))
        userLevelText = tr("Administrator");
    else if (userLevel.testFlag(ServerInfo_User::IsModerator))
        userLevelText = tr("Moderator");
    else if (userLevel.testFlag(ServerInfo_User::IsRegistered))
        userLevelText = tr("Registered user");
    else
        userLevelText = tr("Unregistered user");

    if (user.has_privlevel() && user.privlevel() != "NONE") {
        userLevelText += " | " + tr("%1").arg(user.privlevel().c_str());
    }

    userLevelLabel3.setText(userLevelText);

    QString accountAgeString = tr("Unregistered user");
    if (userLevel.testFlag(ServerInfo_User::IsAdmin) || userLevel.testFlag(ServerInfo_User::IsModerator) || userLevel.testFlag(ServerInfo_User::IsRegistered)) {
        if (user.accountage_secs() == 0)
            accountAgeString = tr("Unknown");
        else {
            qint64 seconds = user.accountage_secs();
            qint64 minutes =  seconds / SIXTY;
            qint64 hours = minutes / SIXTY;
            qint64 days = hours / HOURS_IN_A_DAY;
            qint64 years = days / DAYS_IN_A_YEAR;
            qint64 daysMinusYears = days - (years * DAYS_IN_A_YEAR);

            accountAgeString = "";
            if (years >= 1) {
                accountAgeString = QString::number(years);
                accountAgeString.append(" ");
                accountAgeString.append(years == 1 ? tr("Year") : tr("Years"));
                accountAgeString.append(" ");
            }

            accountAgeString.append(QString::number(daysMinusYears));
            accountAgeString.append(" ");
            accountAgeString.append(days == 1 ? tr("Day") : tr("Days"));
        }
    }
    accountAgeLabel2.setText(accountAgeString);
}

void UserInfoBox::updateInfo(const QString &userName)
{
    Command_GetUserInfo cmd;
    cmd.set_user_name(userName.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(processResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::processResponse(const Response &r)
{
    const Response_GetUserInfo &response = r.GetExtension(Response_GetUserInfo::ext);
    updateInfo(response.user_info());
    setFixedSize(sizeHint());
    show();
}

void UserInfoBox::actEdit()
{
    Command_GetUserInfo cmd;

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(actEditInternal(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::actEditInternal(const Response &r)
{
    const Response_GetUserInfo &response = r.GetExtension(Response_GetUserInfo::ext);
    const ServerInfo_User &user = response.user_info();

    QString email = QString::fromStdString(user.email());
    QString country = QString::fromStdString(user.country());
    QString realName = QString::fromStdString(user.real_name());

    DlgEditUser dlg(this, email, country, realName);
    if(!dlg.exec())
        return;

    Command_AccountEdit cmd;
    cmd.set_real_name(dlg.getRealName().toStdString());
    cmd.set_email(dlg.getEmail().toStdString());
    cmd.set_gender((ServerInfo_User_Gender) dlg.getGender());
    cmd.set_country(dlg.getCountry().toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(processEditResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::actPassword()
{
    DlgEditPassword dlg(this);
    if(!dlg.exec())
        return;

    Command_AccountPassword cmd;
    cmd.set_old_password(dlg.getOldPassword().toStdString());
    cmd.set_new_password(dlg.getNewPassword().toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(processPasswordResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::actAvatar()
{
    DlgEditAvatar dlg(this);
    if(!dlg.exec())
        return;

    Command_AccountImage cmd;
    cmd.set_image(dlg.getImage().data(), dlg.getImage().size());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(processAvatarResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::processEditResponse(const Response &r)
{
    switch (r.response_code()) {
        case Response::RespOk:
            updateInfo(nameLabel.text());
            QMessageBox::information(this, tr("Information"), tr("User information updated."));
            break;
        case Response::RespFunctionNotAllowed:
            QMessageBox::critical(this, tr("Error"), tr("This server does not permit you to update your user informations."));
            break;
        case Response::RespInternalError:
        default:
            QMessageBox::critical(this, tr("Error"), tr("An error occured while trying to update your user informations."));
            break;
    }
}

void UserInfoBox::processPasswordResponse(const Response &r)
{
    switch (r.response_code()) {
        case Response::RespOk:
            QMessageBox::information(this, tr("Information"), tr("Password changed."));
            break;
        case Response::RespFunctionNotAllowed:
            QMessageBox::critical(this, tr("Error"), tr("This server does not permit you to change your password."));
            break;
        case Response::RespPasswordTooShort:
            QMessageBox::critical(this, tr("Error"), tr("The new password is too short."));
            break;
        case Response::RespWrongPassword:
            QMessageBox::critical(this, tr("Error"), tr("The old password is incorrect."));
            break;
        case Response::RespInternalError:
        default:
            QMessageBox::critical(this, tr("Error"), tr("An error occured while trying to update your user informations."));
            break;
    }
}

void UserInfoBox::processAvatarResponse(const Response &r)
{
    switch (r.response_code()) {
        case Response::RespOk:
            updateInfo(nameLabel.text());
            QMessageBox::information(this, tr("Information"), tr("Avatar updated."));
            break;
        case Response::RespFunctionNotAllowed:
            QMessageBox::critical(this, tr("Error"), tr("This server does not permit you to update your avatar."));
            break;
        case Response::RespInternalError:
        default:
            QMessageBox::critical(this, tr("Error"), tr("An error occured while trying to updater your avatar."));
            break;
    }
}
