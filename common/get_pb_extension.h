#ifndef GET_PB_EXTENSION_H
#define GET_PB_EXTENSION_H

namespace google {
	namespace protobuf {
		class Message;
	}
}

int getPbExtension(const ::google::protobuf::Message &message);

#endif
