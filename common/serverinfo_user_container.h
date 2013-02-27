#ifndef SERVERINFO_USER_CONTAINER
#define SERVERINFO_USER_CONTAINER

class ServerInfo_User;

class ServerInfo_User_Container {
protected:
	ServerInfo_User *userInfo;
public:
	ServerInfo_User_Container(ServerInfo_User *_userInfo = 0);
	ServerInfo_User_Container(const ServerInfo_User &_userInfo);
	ServerInfo_User_Container(const ServerInfo_User_Container &other);
	virtual ~ServerInfo_User_Container();
	ServerInfo_User *getUserInfo() const { return userInfo; }
	void setUserInfo(const ServerInfo_User &_userInfo);
	ServerInfo_User &copyUserInfo(ServerInfo_User &result, bool complete, bool internalInfo = false, bool sessionInfo = false) const;
	ServerInfo_User copyUserInfo(bool complete, bool internalInfo = false, bool sessionInfo = false) const;
};

#endif
