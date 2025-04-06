#ifndef SERVERINFO_USER_CONTAINER
#define SERVERINFO_USER_CONTAINER

class ServerInfo_User;

class ServerInfo_User_Container
{
protected:
    ServerInfo_User *userInfo;

public:
    explicit ServerInfo_User_Container(ServerInfo_User *_userInfo = nullptr);
    explicit ServerInfo_User_Container(const ServerInfo_User &_userInfo);
    ServerInfo_User_Container(const ServerInfo_User_Container &other);
    ServerInfo_User_Container &operator=(const ServerInfo_User_Container &other) = default;
    virtual ~ServerInfo_User_Container();
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }
    void setUserInfo(const ServerInfo_User &_userInfo);
    ServerInfo_User &
    copyUserInfo(ServerInfo_User &result, bool complete, bool internalInfo = false, bool sessionInfo = false) const;
    ServerInfo_User copyUserInfo(bool complete, bool internalInfo = false, bool sessionInfo = false) const;
};

#endif
