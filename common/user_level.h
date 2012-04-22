#ifndef USER_LEVEL_H
#define USER_LEVEL_H

#include "pb/serverinfo_user.pb.h"
#include <QFlags>

Q_DECLARE_FLAGS(UserLevelFlags, ServerInfo_User::UserLevelFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(UserLevelFlags)

#endif
