#include "holder_line.h"
#include "restart_manager_holders.h"

#include <string>
#include <vector>
#include <windows.h>

namespace
{
/// The installer sets this, the same variable the running-instance guard uses, instead
/// of passing the directory on the command line: a path with spaces, quotes or a
/// trailing backslash then never has to survive NSIS quoting or a command line.
constexpr wchar_t INSTALL_DIR_VARIABLE[] = L"COCKATRICE_GUARD_DIR";

/// Exit codes the installer reads. "Something was found" is deliberately not 0, so an
/// answer of "nothing holds the files" stays distinguishable from a probe that failed.
enum ExitCode
{
    EXIT_NO_HOLDERS = 0,
    EXIT_HOLDERS_FOUND = 1,
    EXIT_NOT_CONFIGURED = 2,
    EXIT_PROBE_FAILED = 3,
};

/// Writes one line to the pipe nsExec reads. Narrowed to ASCII on purpose: the bytes
/// come back to the installer as an ANSI string, so a non-ASCII process name would be
/// mojibake in the log; a '?' at least stays honest. Control characters are dropped
/// too, so a name can never forge a second log line.
void writeLine(const std::wstring &text)
{
    std::string narrow;
    narrow.reserve(text.size() + 2);
    for (const wchar_t character : text) {
        narrow.push_back(character >= 0x20 && character < 0x7F ? static_cast<char>(character) : '?');
    }
    narrow += "\r\n";
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), narrow.data(), static_cast<DWORD>(narrow.size()), &written, nullptr);
}

std::wstring installDirFromEnvironment()
{
    for (DWORD size = MAX_PATH; size <= 32768U; size *= 2) {
        std::vector<wchar_t> buffer(size);
        const DWORD length = GetEnvironmentVariableW(INSTALL_DIR_VARIABLE, buffer.data(), size);
        if (length == 0) {
            return {};
        }
        if (length < size) {
            return std::wstring(buffer.data(), length);
        }
    }
    return {};
}
} // namespace

int main()
{
    const std::wstring installDir = installDirFromEnvironment();
    if (installDir.empty()) {
        writeLine(L"the probe was not given the install directory");
        return EXIT_NOT_CONFIGURED;
    }

    std::vector<Holder> holders;
    const unsigned long error = findProcessesUsingFiles(collectProbeFiles(installDir), holders);
    if (error != ERROR_SUCCESS) {
        writeLine(L"the Restart Manager lookup failed with win32 error " + std::to_wstring(error));
        return EXIT_PROBE_FAILED;
    }
    if (holders.empty()) {
        // No output: the installer words the "nothing is holding them" case, and it
        // owns the message either way.
        return EXIT_NO_HOLDERS;
    }
    writeLine(formatHolders(holders));
    return EXIT_HOLDERS_FOUND;
}
