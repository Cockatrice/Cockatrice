#include "installer_close_request.h"

#include "installer_close_name.h"

// clang-format off
// windows.h has to come before anything that pulls in a Windows type, as everywhere in
// this tool.
#include <windows.h>
// clang-format on

#include <cstddef>
#include <libcockatrice/utility/installer_close_protocol.h>
#include <string>
#include <vector>

namespace
{
/// How long a busy pipe may take before this gives up. The client answers in milliseconds;
/// the wait only covers a machine under load.
constexpr DWORD PIPE_WAIT_MILLISECONDS = 1000;

std::wstring environmentVariable(const wchar_t *name)
{
    for (DWORD size = MAX_PATH; size <= 32768U; size *= 2) {
        std::vector<wchar_t> buffer(size);
        const DWORD length = GetEnvironmentVariableW(name, buffer.data(), size);
        if (length == 0) {
            return {};
        }
        if (length < size) {
            return std::wstring(buffer.data(), length);
        }
    }
    return {};
}

/// The account name scopedLocalServerName() appends: USER, then USERNAME, then the last
/// component of the profile path - the same order and the same fallbacks the client uses.
std::wstring currentUserName()
{
    for (const wchar_t *name : {L"USER", L"USERNAME"}) {
        const std::wstring value = environmentVariable(name);
        if (!value.empty()) {
            return value;
        }
    }
    const std::wstring profile = environmentVariable(L"USERPROFILE");
    const std::size_t separator = profile.find_last_of(L"\\/");
    if (separator == std::wstring::npos) {
        return profile;
    }
    return profile.substr(separator + 1);
}

/// The named pipe QLocalServer creates for the client's socket, or empty when the account
/// name could not be read and there is nothing to connect to.
std::wstring pipePath(unsigned long processId)
{
    const std::wstring userName = currentUserName();
    if (userName.empty()) {
        return {};
    }
    return L"\\\\.\\pipe\\" + installerCloseSocketName(processId, userName);
}
} // namespace

bool requestInstallerClose(unsigned long processId)
{
    const std::wstring path = pipePath(processId);
    if (path.empty()) {
        return false;
    }
    // A client that is still serving the previous connection answers ERROR_PIPE_BUSY; one
    // retry covers that, while a client that is not listening at all (an older version, or
    // one that has already exited) is answered with a plain failure and the caller falls
    // back to WM_CLOSE.
    for (int attempt = 0; attempt < 2; ++attempt) {
        const HANDLE pipe = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (pipe == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_PIPE_BUSY && WaitNamedPipeW(path.c_str(), PIPE_WAIT_MILLISECONDS) != FALSE) {
                continue;
            }
            return false;
        }
        DWORD written = 0;
        const BOOL ok =
            WriteFile(pipe, INSTALLER_CLOSE_COMMAND, sizeof(INSTALLER_CLOSE_COMMAND) - 1, &written, nullptr);
        CloseHandle(pipe);
        // The request is in the pipe buffer, so the client reads it even though this handle
        // is closed immediately. No acknowledgment is waited for: the caller only needs to
        // know whether there was a listening instance to ask, and a hung client must not
        // stall the guard.
        return ok != FALSE && written == sizeof(INSTALLER_CLOSE_COMMAND) - 1;
    }
    return false;
}
