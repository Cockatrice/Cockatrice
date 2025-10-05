#ifndef DEBUG_PB_MESSAGE_H
#define DEBUG_PB_MESSAGE_H

class QString;
namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google

QString getSafeDebugString(const ::google::protobuf::Message &message);

#endif // DEBUG_PB_MESSAGE_H
