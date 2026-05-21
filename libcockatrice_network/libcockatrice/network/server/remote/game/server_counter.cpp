#include "server_counter.h"

#include <libcockatrice/protocol/pb/serverinfo_counter.pb.h>
#include <limits>

Server_Counter::Server_Counter(int _id, const QString &_name, const color &_counterColor, int _radius, int _count)
    : id(_id), name(_name), counterColor(_counterColor), radius(_radius), count(_count)
{
}

//! \todo Extract overflow-safe arithmetic into shared helper.
//! Duplicated in Server_Card::incrementCounter() - keep in sync if modified.
bool Server_Counter::incrementCount(int delta)
{
    const int oldCount = count;
    const auto result = static_cast<int64_t>(count) + static_cast<int64_t>(delta);
    count = static_cast<int>(qBound(static_cast<int64_t>(std::numeric_limits<int>::min()), result,
                                    static_cast<int64_t>(std::numeric_limits<int>::max())));
    return count != oldCount;
}

void Server_Counter::getInfo(ServerInfo_Counter *info)
{
    info->set_id(id);
    info->set_name(name.toStdString());
    info->mutable_counter_color()->CopyFrom(counterColor);
    info->set_radius(radius);
    info->set_count(count);
}
