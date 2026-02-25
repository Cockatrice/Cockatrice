#include "server_counter.h"

#include <libcockatrice/protocol/pb/serverinfo_counter.pb.h>

Server_Counter::Server_Counter(int _id,
                               const QString &_name,
                               const color &_counterColor,
                               int _radius,
                               int _count,
                               int _minValue,
                               int _maxValue)
    : id(_id), name(_name), counterColor(_counterColor), radius(_radius), count(qBound(_minValue, _count, _maxValue)),
      minValue(_minValue), maxValue(_maxValue)
{
}

void Server_Counter::getInfo(ServerInfo_Counter *info)
{
    info->set_id(id);
    info->set_name(name.toStdString());
    info->mutable_counter_color()->CopyFrom(counterColor);
    info->set_radius(radius);
    info->set_count(count);
}
