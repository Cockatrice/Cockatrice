#include "debug_pb_message.h"

#include "stringsizes.h"

#include <QList>
#include <QString>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

// FastFieldValuePrinter is added in protobuf 3.4, going out of our way to add the old FieldValuePrinter is not worth it
#if GOOGLE_PROTOBUF_VERSION > 3004000

// value printer to use for all values, will snip too long contents
class LimitedPrinter : public ::google::protobuf::TextFormat::FastFieldValuePrinter
{
public:
    void PrintString(const std::string &val,
                     ::google::protobuf::TextFormat::BaseTextGenerator *generator) const override;
};

// value printer to use for specifc values, will expunge sensitive info
class SafePrinter : public ::google::protobuf::TextFormat::FastFieldValuePrinter
{
public:
    void PrintString(const std::string &val,
                     ::google::protobuf::TextFormat::BaseTextGenerator *generator) const override;

    static void applySafePrinter(const ::google::protobuf::Message &message,
                                 ::google::protobuf::TextFormat::Printer &printer);
};

void LimitedPrinter::PrintString(const std::string &val,
                                 ::google::protobuf::TextFormat::BaseTextGenerator *generator) const
{
    auto length = val.length();
    if (length > MAX_TEXT_LENGTH) {
        ::google::protobuf::TextFormat::FastFieldValuePrinter::PrintString(
            val.substr(0, MAX_NAME_LENGTH) + "... ---snip--- (" + std::to_string(length) + " bytes total", generator);
    } else {
        ::google::protobuf::TextFormat::FastFieldValuePrinter::PrintString(val, generator);
    }
}

void SafePrinter::PrintString(const std::string & /*val*/,
                              ::google::protobuf::TextFormat::BaseTextGenerator *generator) const
{
    generator->PrintLiteral("\" ---value expunged--- \"");
}

void SafePrinter::applySafePrinter(const ::google::protobuf::Message &message,
                                   ::google::protobuf::TextFormat::Printer &printer)
{
    const auto *reflection = message.GetReflection();
    std::vector<const google::protobuf::FieldDescriptor *> fields;
    reflection->ListFields(message, &fields);
    for (const auto *field : fields) {
        switch (field->cpp_type()) {
            case ::google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                if (field->name().find("password") != std::string::npos) { // name contains password
                    auto *safePrinter = new SafePrinter();
                    if (!printer.RegisterFieldValuePrinter(field, safePrinter))
                        delete safePrinter; // in case safePrinter has not been taken ownership of
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                if (field->is_repeated()) {
                    for (int i = 0; i < reflection->FieldSize(message, field); ++i) {
                        applySafePrinter(reflection->GetRepeatedMessage(message, field, i), printer);
                    }
                } else {
                    applySafePrinter(reflection->GetMessage(message, field), printer);
                }
                break;
            default:
                break;
        }
    }
}
#endif // GOOGLE_PROTOBUF_VERSION > 3004000

QString getSafeDebugString(const ::google::protobuf::Message &message)
{
#if GOOGLE_PROTOBUF_VERSION > 3001000
    auto size = message.ByteSizeLong();
#else
    auto size = message.ByteSize();
#endif

    ::google::protobuf::TextFormat::Printer printer;
    printer.SetSingleLineMode(true); // compact mode
    printer.SetExpandAny(true);      // prints all fields

#if GOOGLE_PROTOBUF_VERSION > 3004000
    // printer takes ownership of the LimitedPrinter and will delete it
    printer.SetDefaultFieldValuePrinter(new LimitedPrinter());
    // check field names an create SafePrinters for necessary fields
    SafePrinter::applySafePrinter(message, printer);
#else
    // removing passwords from debug output will only be supported on newer protobuf versions
    printer.SetTruncateStringFieldLongerThan(MAX_TEXT_LENGTH);
#endif // GOOGLE_PROTOBUF_VERSION > 3004000

    std::string debug_string;
    printer.PrintToString(message, &debug_string);
    return QString::number(size) + " bytes " + QString::fromStdString(debug_string);
}
