#include "userinfobox.h"

#include "abstractclient.h"
#include "dlg_edit_avatar.h"
#include "dlg_edit_password.h"
#include "dlg_edit_user.h"
#include "gettextwithmax.h"
#include "passwordhasher.h"
#include "pb/response_get_user_info.pb.h"
#include "pb/session_commands.pb.h"
#include "pending_command.h"
#include "pixmapgenerator.h"

#include <QDateTime>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>

UserInfoBox::UserInfoBox(AbstractClient *_client, bool _editable, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags), client(_client), editable(_editable)
{
    QFont nameFont = nameLabel.font();
    nameFont.setBold(true);
    nameFont.setPointSizeF(nameFont.pointSizeF() * 1.5);
    nameLabel.setFont(nameFont);

    auto *userIconAndNameLayout = new QHBoxLayout;
    userIconAndNameLayout->addWidget(&userLevelIcon, 0, Qt::AlignCenter);
    userIconAndNameLayout->addWidget(&nameLabel, 1);
    userIconAndNameLayout->addStretch();

    avatarPic.setMinimumSize(200, 200);
    avatarPic.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    avatarPic.setAlignment(Qt::AlignCenter);

    auto *avatarLayout = new QHBoxLayout;
    avatarLayout->setContentsMargins(0, 0, 0, 0);
    avatarLayout->addStretch(1);
    avatarLayout->addWidget(&avatarPic, 3);
    avatarLayout->addStretch(1);

    auto *mainLayout = new QGridLayout;
    mainLayout->addLayout(avatarLayout, 0, 0, 1, 3);
    mainLayout->addLayout(userIconAndNameLayout, 1, 0, 1, 3);
    mainLayout->addWidget(&realNameLabel1, 2, 0, 1, 1);
    mainLayout->addWidget(&realNameLabel2, 2, 1, 1, 2);
    mainLayout->addWidget(&countryLabel1, 3, 0, 1, 1);
    mainLayout->addWidget(&countryLabel2, 3, 1, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(&countryLabel3, 3, 2, 1, 1);
    mainLayout->addWidget(&userLevelLabel1, 4, 0, 1, 1);
    mainLayout->addWidget(&userLevelLabel2, 4, 1, 1, 2);
    mainLayout->addWidget(&accountAgeLabel1, 5, 0, 1, 1);
    mainLayout->addWidget(&accountAgeLabel2, 5, 1, 1, 2);
    mainLayout->setColumnStretch(2, 10);

    if (editable) {
        auto *buttonsLayout = new QHBoxLayout;
        buttonsLayout->addWidget(&editButton);
        buttonsLayout->addWidget(&passwordButton);
        buttonsLayout->addWidget(&avatarButton);
        mainLayout->addLayout(buttonsLayout, 7, 0, 1, 3);

        connect(&editButton, SIGNAL(clicked()), this, SLOT(actEdit()));
        connect(&passwordButton, SIGNAL(clicked()), this, SLOT(actPassword()));
        connect(&avatarButton, SIGNAL(clicked()), this, SLOT(actAvatar()));
    }

    setWindowTitle(tr("User Information"));
    setLayout(mainLayout);
    retranslateUi();
}

void UserInfoBox::retranslateUi()
{
    realNameLabel1.setText(tr("Real Name:"));
    countryLabel1.setText(tr("Location:"));
    userLevelLabel1.setText(tr("User Level:"));
    accountAgeLabel1.setText(tr("Account Age:"));

    editButton.setText(tr("Edit"));
    passwordButton.setText(tr("Change password"));
    avatarButton.setText(tr("Change avatar"));
}

void UserInfoBox::updateInfo(const ServerInfo_User &user)
{
    const UserLevelFlags userLevel(user.user_level());

    const std::string &bmp = user.avatar_bmp();
    if (!avatarPixmap.loadFromData((const uchar *)bmp.data(), static_cast<uint>(bmp.size()))) {
        avatarPixmap =
            UserLevelPixmapGenerator::generatePixmap(64, userLevel, false, QString::fromStdString(user.privlevel()));
    }

    nameLabel.setText(QString::fromStdString(user.name()));
    realNameLabel2.setText(QString::fromStdString(user.real_name()));
    QString country = QString::fromStdString(user.country());

    if (country.length() != 0) {
        countryLabel2.setPixmap(CountryPixmapGenerator::generatePixmap(13, country));
        countryLabel3.setText(QString("%1").arg(country.toUpper()));
    } else {
        countryLabel2.setText("");
        countryLabel3.setText("");
    }

    userLevelIcon.setPixmap(
        UserLevelPixmapGenerator::generatePixmap(15, userLevel, false, QString::fromStdString(user.privlevel())));
    QString userLevelText;
    if (userLevel.testFlag(ServerInfo_User::IsAdmin))
        userLevelText = tr("Administrator");
    else if (userLevel.testFlag(ServerInfo_User::IsModerator))
        userLevelText = tr("Moderator");
    else if (userLevel.testFlag(ServerInfo_User::IsRegistered))
        userLevelText = tr("Registered user");
    else
        userLevelText = tr("Unregistered user");

    if (userLevel.testFlag(ServerInfo_User::IsJudge))
        userLevelText += " | " + tr("Judge");

    if (user.has_privlevel() && user.privlevel() != "NONE") {
        userLevelText += " | " + QString("%1").arg(user.privlevel().c_str());
    }

    userLevelLabel2.setText(userLevelText);

    QString accountAgeString = tr("Unregistered user");
    if (userLevel.testFlag(ServerInfo_User::IsAdmin) || userLevel.testFlag(ServerInfo_User::IsModerator) ||
        userLevel.testFlag(ServerInfo_User::IsRegistered)) {
        accountAgeString = getAgeString(user.accountage_secs());
    }
    accountAgeLabel2.setText(accountAgeString);
}

QString UserInfoBox::getAgeString(int ageSeconds)
{
    QString accountAgeString = tr("Unknown");
    if (ageSeconds == 0)
        return accountAgeString;

    auto date = QDateTime::fromSecsSinceEpoch(QDateTime::currentSecsSinceEpoch() - ageSeconds).date();
    if (!date.isValid())
        return accountAgeString;

    QString dateString = QLocale().toString(date, QLocale::ShortFormat);
    auto now = QDate::currentDate();
    auto daysAndYears = getDaysAndYearsBetween(date, now);

    QString yearString;
    if (daysAndYears.second > 0) {
        yearString = tr("%n Year(s), ", "amount of years (only shown if more than 0)", daysAndYears.second);
    }
    accountAgeString =
        tr("%10%n Day(s) %20", "amount of years (if more than 0), amount of days, date in local short format",
           daysAndYears.first)
            .arg(yearString)
            .arg(dateString);
    return accountAgeString;
}

void UserInfoBox::updateInfo(const QString &userName)
{
    Command_GetUserInfo cmd;
    cmd.set_user_name(userName.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(processResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::processResponse(const Response &r)
{
    const Response_GetUserInfo &response = r.GetExtension(Response_GetUserInfo::ext);
    updateInfo(response.user_info());
    resize(sizeHint());
    show();
}

void UserInfoBox::actEdit()
{
    Command_GetUserInfo cmd;

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(actEditInternal(const Response &)));

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
    if (!dlg.exec())
        return;

    Command_AccountEdit cmd;
    cmd.set_real_name(dlg.getRealName().toStdString());
    if (client->getServerSupportsPasswordHash()) {
        if (email != dlg.getEmail()) {
            // real password is required to change email
            bool ok = false;
            QString password =
                getTextWithMax(this, tr("Enter Password"),
                               tr("Password verification is required in order to change your email address"),
                               QLineEdit::Password, "", &ok);
            if (!ok)
                return;
            cmd.set_password_check(password.toStdString());
            cmd.set_email(dlg.getEmail().toStdString());
        } // servers that support password hash do not require all fields to be filled anymore
    } else {
        cmd.set_email(dlg.getEmail().toStdString());
    }
    cmd.set_country(dlg.getCountry().toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(processEditResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::actPassword()
{
    DlgEditPassword dlg(this);
    if (!dlg.exec())
        return;

    auto oldPassword = dlg.getOldPassword();
    auto newPassword = dlg.getNewPassword();

    if (client->getServerSupportsPasswordHash()) {
        Command_RequestPasswordSalt cmd;
        cmd.set_user_name(client->getUserName().toStdString());

        PendingCommand *pend = client->prepareSessionCommand(cmd);
        connect(pend,
                // we need qoverload here in order to select the right version of this function
                QOverload<const Response &, const CommandContainer &, const QVariant &>::of(&PendingCommand::finished),
                this, [=](const Response &response, const CommandContainer &, const QVariant &) {
                    if (response.response_code() == Response::RespOk) {
                        changePassword(oldPassword, newPassword);
                    } else {
                        QMessageBox::critical(this, tr("Error"),
                                              tr("An error occurred while trying to update your user information."));
                    }
                });
        client->sendCommand(pend);
    } else {
        changePassword(oldPassword, newPassword);
    }
}

void UserInfoBox::changePassword(const QString &oldPassword, const QString &newPassword)
{
    Command_AccountPassword cmd;
    cmd.set_old_password(oldPassword.toStdString());
    if (client->getServerSupportsPasswordHash()) {
        auto passwordSalt = PasswordHasher::generateRandomSalt();
        QString hashedPassword = PasswordHasher::computeHash(newPassword, passwordSalt);
        cmd.set_hashed_new_password(hashedPassword.toStdString());
    } else {
        cmd.set_new_password(newPassword.toStdString());
    }

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(processPasswordResponse(const Response &)));

    client->sendCommand(pend);
}

void UserInfoBox::actAvatar()
{
    DlgEditAvatar dlg(this);
    if (!dlg.exec())
        return;

    Command_AccountImage cmd;
    cmd.set_image(dlg.getImage().data(), dlg.getImage().size());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(processAvatarResponse(const Response &)));

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
            QMessageBox::critical(this, tr("Error"),
                                  tr("This server does not permit you to update your user informations."));
            break;
        case Response::RespWrongPassword:
            QMessageBox::critical(this, tr("Error"), tr("The entered password does not match your account."));
            break;
        case Response::RespInternalError:
        default:
            QMessageBox::critical(this, tr("Error"),
                                  tr("An error occurred while trying to update your user information."));
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
            QMessageBox::critical(this, tr("Error"),
                                  tr("An error occurred while trying to update your user information."));
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

void UserInfoBox::resizeEvent(QResizeEvent *event)
{
    QPixmap resizedPixmap = avatarPixmap.scaled(avatarPic.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    avatarPic.setPixmap(resizedPixmap);
    QWidget::resizeEvent(event);
}
