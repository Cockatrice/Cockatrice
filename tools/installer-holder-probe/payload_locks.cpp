#include "payload_locks.h"

#include "payload_paths.h"

#include <windows.h>

namespace
{
/// How long the second look waits before it re-tests the files that refused the write open.
/// Only those are re-tested, so the one answer costs this at most once: a scanner, or a
/// program that is only starting, holds a file without write sharing for a moment, and that
/// must not turn a free payload into a run the user has to answer.
constexpr DWORD RECHECK_DELAY_MILLISECONDS = 300;

/// Whether @p path refuses an open that asks to write it.
///
/// Windows opens a program's image, and every module loaded into it, with read and delete
/// sharing but not write sharing, so this is refused for as long as the program runs and by
/// nothing else that merely reads the file. A read-only file is skipped: it refuses a write
/// open for its own reason and would answer yes for a payload nobody is running, and the
/// purge clears that attribute before it deletes anything. Any other error is no answer at
/// all - access this probe does not have says nothing about whether the file is in use, and
/// still answering "in use" would stop installs that could have finished.
bool refusesWrite(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)) != 0) {
        return false;
    }
    const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return GetLastError() == ERROR_SHARING_VIOLATION;
    }
    // Nothing was written: the open is the whole test, and closing the handle leaves the
    // file exactly as it was.
    CloseHandle(file);
    return false;
}
} // namespace

std::vector<std::wstring> payloadExecutablesInUse(const std::wstring &installDir)
{
    std::vector<std::wstring> refused;
    for (const wchar_t *name : PAYLOAD_PROGRAMS) {
        const std::wstring path = installDir + L"\\" + name;
        if (refusesWrite(path)) {
            refused.push_back(path);
        }
    }
    if (refused.empty()) {
        return refused;
    }

    // One re-test of the files that refused, and only those: the answer is shown to a user
    // who then has to close a program and run the installer again, so it has to be the
    // settled state rather than a moment in which something happened to hold the file.
    Sleep(RECHECK_DELAY_MILLISECONDS);
    std::vector<std::wstring> settled;
    for (const std::wstring &path : refused) {
        if (refusesWrite(path)) {
            settled.push_back(path);
        }
    }
    return settled;
}
