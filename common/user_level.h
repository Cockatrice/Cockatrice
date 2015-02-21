#ifndef USER_LEVEL_H
#define USER_LEVEL_H

#ifdef Q_OS_OSX
// avoid collision from Mac OS X's ConditionalMacros.h
// https://code.google.com/p/protobuf/issues/detail?id=119
#undef TYPE_BOOL
#endif
#include "pb/serverinfo_user.pb.h"
#include <QFlags>

Q_DECLARE_FLAGS(UserLevelFlags, ServerInfo_User::UserLevelFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(UserLevelFlags)

#endif
