#include "close_holders.h"
#include "holder_line.h"
#include "restart_manager_holders.h"

#include <algorithm>
#include <string>
#include <vector>
#include <windows.h>

namespace
{
/// The installer sets this, the same variable the running-instance guard uses, instead
/// of passing the directory on the command line: a path with spaces, quotes or a
/// trailing backslash then never has to survive NSIS quoting or a command line.
constexpr wchar_t INSTALL_DIR_VARIABLE[] = L"COCKATRICE_GUARD_DIR";

/// How long the close mode waits for the instance it asked to exit before it reports the
/// process as still running. The installer measures the purge right after this call, so
/// the wait belongs here: it is the only place that holds a handle on the process whose
/// exit it is waiting for.
constexpr unsigned long CLOSE_WAIT_MILLISECONDS = 30000;

/// Exit codes the installer reads. "Something was found" is deliberately not 0, so an
/// answer of "nothing holds the files" stays distinguishable from a probe that failed.
/// In the close mode 1 means a close was actually sent and 0 that every holder was left
/// to the user, which is what the report line next to the code spells out. 3 is only for
/// a lookup that failed and named nobody: one that failed but still named a process
/// running from the directory is answered with that process.
enum ExitCode
{
    EXIT_NO_HOLDERS = 0,
    EXIT_HOLDERS_FOUND = 1,
    EXIT_NOT_CONFIGURED = 2,
    EXIT_PROBE_FAILED = 3,
};

/// The probe has two modes, selected by the first argument:
///
///   (none)           name what holds the payload, one line, for the guard prompt and the
///                     abort dialog/log
///   --close-holders  bring the running Cockatrice forward, ask it to close, and wait for
///                     it to exit, for the offer to close after a failed purge
///
/// Both answer from two lookups that cover each other's blind spot: the Restart Manager
/// names whoever has a payload file open, and every process running from the directory is
/// named as well, because a running instance holds its own image and the modules it
/// loaded as mappings rather than open files and the Restart Manager does not always
/// report those. Neither lookup alone is the guard's authority: an instance the file
/// handles do not name is exactly the case that left a half-replaced install behind.
bool closeModeRequested(int argumentCount, char **arguments)
{
    return argumentCount > 1 && std::string(arguments[1]) == "--close-holders";
}

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

int main(int argumentCount, char **arguments)
{
    const std::wstring installDir = installDirFromEnvironment();
    if (installDir.empty()) {
        writeLine(L"the probe was not given the install directory");
        return EXIT_NOT_CONFIGURED;
    }

    // Processes running from the directory first: they are the instance the user is asked
    // to close, and the Restart Manager may not name any of them (see
    // findProcessesRunningFrom). The file holders follow, deduplicated by process, so an
    // instance that both runs from the directory and holds a payload file is named once.
    std::vector<Holder> holders = findProcessesRunningFrom(installDir);
    std::vector<Holder> fileHolders;
    const unsigned long error = findProcessesUsingFiles(collectPayloadFiles(installDir), fileHolders);
    for (const Holder &holder : fileHolders) {
        const bool alreadyNamed = std::any_of(holders.begin(), holders.end(), [&holder](const Holder &named) {
            return named.processId == holder.processId;
        });
        if (!alreadyNamed) {
            holders.push_back(holder);
        }
    }

    // A failed lookup only makes the probe unable to answer when it also named nobody: with
    // a process running from the directory the installer still knows what to close, and
    // that is worth more to the guard than the code of the query that missed it.
    if (error != ERROR_SUCCESS && holders.empty()) {
        writeLine(L"the Restart Manager lookup failed with win32 error " + std::to_wstring(error));
        return EXIT_PROBE_FAILED;
    }

    const bool closeMode = closeModeRequested(argumentCount, arguments);
    if (holders.empty()) {
        // No output in the naming mode: the installer words the "nothing is holding them"
        // case, and it owns the message either way. The close mode says it itself, because
        // its line is logged verbatim next to what it was asked to do.
        if (closeMode) {
            writeLine(L"nothing is holding the files");
        }
        return EXIT_NO_HOLDERS;
    }
    if (!closeMode) {
        writeLine(formatHolders(holders));
        return EXIT_HOLDERS_FOUND;
    }

    const CloseHoldersResult result = closeCockatriceHolders(holders, CLOSE_WAIT_MILLISECONDS);
    writeLine(result.report);
    return result.askedToClose > 0 ? EXIT_HOLDERS_FOUND : EXIT_NO_HOLDERS;
}
