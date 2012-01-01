#include "get_pb_extension.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

int getPbExtension(const ::google::protobuf::Message &message)
{
	std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
	message.GetReflection()->ListFields(message, &fieldList);
	for (unsigned int j = 0; j < fieldList.size(); ++j)
		if (fieldList[j]->is_extension())
			return fieldList[j]->number();
	return -1;
}
