#include "payload_paths.h"

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace
{
/// The payload programs, by image name. A process running one of these is what the guard
/// exists to name, so askingProcessIds() stops its walk up the parent chain at one.
constexpr const wchar_t *PAYLOAD_PROGRAMS[] = {L"cockatrice.exe", L"oracle.exe", L"servatrice.exe"};

/// Compares two file names the way the file system does, without a locale.
///
/// Only the names above are ever compared against, and all of them are ASCII, so folding
/// the ASCII letters is the whole answer. The Windows ordinal comparison is not used here
/// on purpose: this file is compiled into a test that runs everywhere, and the question
/// "is this image a program the payload installs?" must be answerable without Windows.
bool equalsIgnoringAsciiCase(const std::wstring &left, const std::wstring &right)
{
    if (left.size() != right.size()) {
        return false;
    }
    return std::equal(left.begin(), left.end(), right.begin(), [](wchar_t a, wchar_t b) {
        const wchar_t foldedA = a >= L'A' && a <= L'Z' ? static_cast<wchar_t>(a - L'A' + L'a') : a;
        const wchar_t foldedB = b >= L'A' && b <= L'Z' ? static_cast<wchar_t>(b - L'A' + L'a') : b;
        return foldedA == foldedB;
    });
}
} // namespace

std::wstring fileNameOf(const std::wstring &path)
{
    const std::size_t separator = path.find_last_of(L"\\/");
    return separator == std::wstring::npos ? path : path.substr(separator + 1);
}

std::wstring parentDirectoryOf(const std::wstring &path)
{
    const std::size_t separator = path.find_last_of(L"\\/");
    return separator == std::wstring::npos ? std::wstring() : path.substr(0, separator);
}

bool isPayloadProgram(const std::wstring &imageName)
{
    for (const wchar_t *program : PAYLOAD_PROGRAMS) {
        if (equalsIgnoringAsciiCase(imageName, program)) {
            return true;
        }
    }
    return false;
}
