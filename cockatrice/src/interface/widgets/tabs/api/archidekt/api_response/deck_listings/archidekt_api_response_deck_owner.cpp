#include "archidekt_api_response_deck_owner.h"

void ArchidektApiResponseDeckOwner::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();
    userName = json.value("username").toString();
    avatar = QUrl(json.value("avatar").toString());
    moderator = json.value("moderator").toBool();
    pledgeLevel = json.value("pledgeLevel").toInt();
    // TODO but not really important
    // roles = {""};
}

void ArchidektApiResponseDeckOwner::debugPrint() const
{
    qDebug() << "Id:" << id;
    qDebug() << "UserName:" << userName;
    qDebug() << "Avatar:" << avatar;
    qDebug() << "Moderator:" << moderator;
    qDebug() << "PledgeLevel:" << pledgeLevel;
    qDebug() << "Roles:" << roles;
}