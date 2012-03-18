#include "serverinfo_user_container.h"
#include "pb/serverinfo_user.pb.h"

ServerInfo_User_Container::ServerInfo_User_Container(ServerInfo_User *_userInfo)
        : userInfo(_userInfo)
{
}

ServerInfo_User_Container::ServerInfo_User_Container(const ServerInfo_User &_userInfo)
	: userInfo(new ServerInfo_User(_userInfo))
{
}

ServerInfo_User_Container::ServerInfo_User_Container(const ServerInfo_User_Container &other)
{
	if (other.userInfo)
		userInfo = new ServerInfo_User(*other.userInfo);
	else
		userInfo = 0;
}

ServerInfo_User_Container::~ServerInfo_User_Container()
{
	delete userInfo;
}

void ServerInfo_User_Container::setUserInfo(const ServerInfo_User &_userInfo)
{
	userInfo = new ServerInfo_User;
	userInfo->CopyFrom(_userInfo);
}

ServerInfo_User ServerInfo_User_Container::copyUserInfo(bool complete, bool moderatorInfo) const
{
	ServerInfo_User result;
	if (userInfo) {
		result.CopyFrom(*userInfo);
		if (!moderatorInfo) {
			result.clear_session_id();
			result.clear_address();
			result.clear_id();
		}
		if (!complete)
			result.clear_avatar_bmp();
	}
	return result;
}
